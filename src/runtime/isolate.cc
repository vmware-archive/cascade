// Copyright 2017-2018 VMware, Inc.
// SPDX-License-Identifier: BSD-2-Clause
//
// The BSD-2 license (the License) set forth below applies to all parts of the
// Cascade project.  You may not use this file except in compliance with the
// License.
//
// BSD-2 License
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/runtime/isolate.h"

#include <cassert>
#include <sstream>
#include <string>
#include "src/base/bits/bits.h"
#include "src/runtime/data_plane.h"
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/program/elaborate.h"
#include "src/verilog/program/inline.h"

using namespace std;

namespace cascade {

Isolate::Isolate(const DataPlane* dp) : Builder() { 
  dp_ = dp;
}

ModuleDeclaration* Isolate::isolate(const ModuleDeclaration* src, int ignore) {
  src_ = src; 
  ignore_ = ignore;
  return src_->accept(this);
}

VId Isolate::isolate(const Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  assert(r != nullptr);

  auto itr = symbol_table_.find(r);
  if (itr == symbol_table_.end()) {
    itr = symbol_table_.insert(make_pair(r, symbol_table_.size()+1)).first;
  }
  return itr->second;
}

Attributes* Isolate::build(const Attributes* as) {
  // Ignore attributes
  (void) as;
  return new Attributes(new Many<AttrSpec>());
}

Expression* Isolate::build(const Identifier* id) {
  if (Resolve().get_resolution(id) == nullptr) {
    return id->clone();
  } else if (ModuleInfo(src_).is_read(id) || ModuleInfo(src_).is_write(id)) { 
    return to_global_id(id);
  } else {
    return to_local_id(id);
  } 
}

ModuleDeclaration* Isolate::build(const ModuleDeclaration* md) {
  // Create a new module with a mangled id and global io ports
  auto res = get_shell();
  // Generate local declarations. 
  res->get_items()->concat(get_local_decls());
  // Transform the remainder of the code
  res->get_items()->concat(get_items(md->get_items(), true));

  return res;
}

ModuleItem* Isolate::build(const InitialConstruct* ic) {
  auto attrs = ic->get_attrs()->accept(this);
  if (ignore_ >= 0) {
    attrs->set_or_replace("__ignore", new String("true"));
  }
  return new InitialConstruct (
    attrs,
    (Statement*) ic->get_stmt()->accept(this)
  );
}

ModuleItem* Isolate::build(const GenvarDeclaration* gd) {
  // Does nothing. We delete these.
  (void) gd;
  return nullptr;
}

ModuleItem* Isolate::build(const IntegerDeclaration* id) {
  auto res = new RegDeclaration(
    id->get_attrs()->accept(this),
    id->get_id()->accept(this),
    true,
    new Maybe<RangeExpression>(new RangeExpression(32,0)),
    id->get_val()->null() ?
      new Maybe<Expression>() :
      new Maybe<Expression>(new Number(Evaluate().get_value(id->get_val()->get()), Number::HEX))
  );
  return res;
}

ModuleItem* Isolate::build(const LocalparamDeclaration* ld) {
  auto res = new LocalparamDeclaration(
    ld->get_attrs()->accept(this),
    ld->get_signed(),
    ld->get_dim()->accept(this),
    ld->get_id()->accept(this),
    new Number(Evaluate().get_value(ld->get_val()), Number::HEX)
  );
  res->get_attrs()->get_as()->push_back(new AttrSpec(
    new Identifier("__id"),
    new Maybe<Expression>(Resolve().get_full_id(ld->get_id())->clone())
  ));
  return res;
}

ModuleItem* Isolate::build(const ParameterDeclaration* pd) {
  // Parameter declarations are downgraded to local params
  auto res = new LocalparamDeclaration(
    pd->get_attrs()->accept(this),
    pd->get_signed(),
    pd->get_dim()->accept(this),
    pd->get_id()->accept(this),
    new Number(Evaluate().get_value(pd->get_val()), Number::HEX)
  );
  res->get_attrs()->get_as()->push_back(new AttrSpec(
    new Identifier("__id"),
    new Maybe<Expression>(Resolve().get_full_id(pd->get_id())->clone())
  ));
  return res;
}

ModuleItem* Isolate::build(const RegDeclaration* rd) {
  auto res = new RegDeclaration(
    rd->get_attrs()->accept(this),
    rd->get_id()->accept(this),
    rd->get_signed(),
    rd->get_dim()->accept(this),
    rd->get_val()->null() ?
      new Maybe<Expression>() :
      new Maybe<Expression>(new Number(Evaluate().get_value(rd->get_val()->get()), Number::HEX))
  );
  return res;
}

ModuleItem* Isolate::build(const PortDeclaration* pd) {
  // Does nothing. We emit declarations at the top level.
  (void) pd;
  return nullptr;
}

Identifier* Isolate::to_mangled_id(const Identifier* id) {
  // Don't use isolate(.) here. We don't want to try to resolve this id.
  auto itr = module_table_.find(id);
  if (itr == module_table_.end()) {
    itr = module_table_.insert(make_pair(id, module_table_.size())).first;
  }

  stringstream ss;
  ss << "__M" << itr->second;
  return new Identifier(ss.str());
}

Identifier* Isolate::to_local_id(const Identifier* id) {
  stringstream ss;
  ss << "__l" << isolate(id);
  return new Identifier(
    new Many<Id>(new Id(ss.str(), new Maybe<Expression>())),
    id->get_dim()->accept(this)
  );
}

Identifier* Isolate::to_global_id(const Identifier* id) {
  stringstream ss;
  ss << "__x" << isolate(id);
  return new Identifier(
    new Many<Id>(new Id(ss.str(), new Maybe<Expression>())),
    id->get_dim()->accept(this)
  );
}

ModuleDeclaration* Isolate::get_shell() {
  ModuleInfo info(src_);

  auto res = new ModuleDeclaration(
    src_->get_attrs()->clone(),
    to_mangled_id(dynamic_cast<ModuleInstantiation*>(src_->get_parent())->get_iid()),
    new Many<ArgAssign>(),
    new Many<ModuleItem>()
  );

  unordered_set<const Identifier*> ports;
  ports.insert(info.reads().begin(), info.reads().end());
  ports.insert(info.writes().begin(), info.writes().end());

  for (auto p : ports) {
    res->get_ports()->push_back(new ArgAssign(
      new Maybe<Identifier>(),                       
      new Maybe<Expression>(to_global_id(p))
    ));

    const auto r = info.is_read(p);
    const auto w = info.is_write(p);
    const auto width = Evaluate().get_width(p);

    // TODO: Is this logic correct? When should a global read/write be promoted
    // to a register and when should it remain a net?

    auto pd = new PortDeclaration(
      new Attributes(new Many<AttrSpec>()), 
      r && w ? PortDeclaration::INOUT : r ? PortDeclaration::INPUT : PortDeclaration::OUTPUT,
      (info.is_local(p) && dynamic_cast<const RegDeclaration*>(p->get_parent())) ? 
        (Declaration*) new RegDeclaration(
          new Attributes(new Many<AttrSpec>()),
          to_global_id(p),
          dynamic_cast<const RegDeclaration*>(p->get_parent())->get_signed(), 
          width == 1 ? new Maybe<RangeExpression>() : new Maybe<RangeExpression>(new RangeExpression(width)),
          dynamic_cast<RegDeclaration*>(p->get_parent())->get_val()->clone()
        ) : 
        (Declaration*) new NetDeclaration(
          new Attributes(new Many<AttrSpec>()),
          NetDeclaration::WIRE,
          new Maybe<DelayControl>(),
          to_global_id(p),
          dynamic_cast<const NetDeclaration*>(p->get_parent())->get_signed(),
          width == 1 ? new Maybe<RangeExpression>() : new Maybe<RangeExpression>(new RangeExpression(width))
        )
    );
    pd->get_attrs()->get_as()->push_back(new AttrSpec(
      new Identifier("__id"), 
      new Maybe<Expression>(Resolve().get_full_id(p)->clone())
    ));
    res->get_items()->push_back(pd);
  }

  return res;
}

Many<ModuleItem>* Isolate::get_local_decls() {
  auto res = new Many<ModuleItem>();
  for (auto l : ModuleInfo(src_).locals()) {
    if (ModuleInfo(src_).is_read(l) || ModuleInfo(src_).is_write(l)) {
      continue;
    }
    auto d = dynamic_cast<const Declaration*>(l->get_parent())->accept(this);
    if (d != nullptr) {
      res->push_back(d);
    }
  }
  return res;
}

Many<ModuleItem>* Isolate::get_items(const Many<ModuleItem>* mis, bool top_level) {
  auto res = new Many<ModuleItem>();
  for (auto mi : *mis) {
    // If this is the top level, we're one step closer to allowing initial constructs
    if (top_level) {
      --ignore_;
    }
    // Ignore Declarations. We handle them at the top level.
    if (dynamic_cast<const Declaration*>(mi)) {
      continue;
    }
    // Flatten generate regions and generate constructs
    else if (auto gr = dynamic_cast<const GenerateRegion*>(mi)) {
      flatten(res, gr);
    } else if (auto cgc = dynamic_cast<const CaseGenerateConstruct*>(mi)) {
      flatten(res, cgc);
    } else if (auto igc = dynamic_cast<const IfGenerateConstruct*>(mi)) {
      flatten(res, igc);
    } else if (auto lgc = dynamic_cast<const LoopGenerateConstruct*>(mi)) {
      flatten(res, lgc);
    }
    // Either descend on instantiations or replace them with connections
    else if (auto inst = dynamic_cast<const ModuleInstantiation*>(mi)) {
      if (Inline().is_inlined(inst)) {
        flatten(res, Inline().get_source(inst));
      } else {
        replace(res, inst); 
      }
    } 
    // Everything else goes through the normal build path. 
    else {
      auto temp = (ModuleItem*) mi->accept(this);
      if (temp != nullptr) {
        res->push_back(temp);
      }
    }
  }
  return res;
}

void Isolate::replace(Many<ModuleItem>* res, const ModuleInstantiation* mi) {
  const auto& conns = ModuleInfo(src_).connections();
  const auto itr = conns.find(mi->get_iid());
  assert(itr != conns.end());

  for (auto& c : itr->second) {
    auto lhs = ModuleInfo(Resolve().get_origin(c.first)).is_input(c.first) ? 
        c.first->accept(this) : 
        (Identifier*)c.second->accept(this);
    auto rhs = ModuleInfo(Resolve().get_origin(c.first)).is_input(c.first) ? 
        (Expression*)c.second->accept(this) : 
        c.first->accept(this);
    auto ca = new ContinuousAssign(
      new Maybe<DelayControl>(),
      new VariableAssign(lhs, rhs)
    );
    res->push_back(ca);
  }
}

void Isolate::flatten(Many<ModuleItem>* res, const CaseGenerateConstruct* cgc) {
  auto elab = Elaborate().get_elaboration(cgc);
  assert(elab != nullptr);
  if (!elab->null()) {
    flatten(res, elab->get());
  }
}

void Isolate::flatten(Many<ModuleItem>* res, const IfGenerateConstruct* igc) {
  auto elab = Elaborate().get_elaboration(igc);
  assert(elab != nullptr);
  if (!elab->null()) {
    flatten(res, elab->get());
  }
}

void Isolate::flatten(Many<ModuleItem>* res, const LoopGenerateConstruct* lgc) {
  auto elab = Elaborate().get_elaboration(lgc);
  assert(elab != nullptr);
  for (auto gb : *elab) {
    flatten(res, gb);
  }
}

void Isolate::flatten(Many<ModuleItem>* res, const GenerateBlock* gb) {
  res->concat(get_items(gb->get_items(), false));
}

void Isolate::flatten(Many<ModuleItem>* res, const GenerateRegion* gr) {
  res->concat(get_items(gr->get_items(), false));
}

} // namespace cascade
