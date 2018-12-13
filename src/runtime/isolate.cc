// Copyright 2017-2019 VMware, Inc.
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
  return new Attributes();
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
  auto ld = get_local_decls();
  res->push_back_items(ld.begin(), ld.end());
  // Transform the remainder of the code
  auto mis = get_items(md->begin_items(), md->end_items(), true);
  res->push_back_items(mis.begin(), mis.end());

  return res;
}

ModuleItem* Isolate::build(const InitialConstruct* ic) {
  auto attrs = ic->get_attrs()->accept(this);
  if (ignore_ >= 0) {
    attrs->set_or_replace("__ignore", new String("true"));
  }
  return new InitialConstruct (
    attrs,
    (Statement*) ic->accept_stmt(this)
  );
}

ModuleItem* Isolate::build(const GenvarDeclaration* gd) {
  // Does nothing. We delete these.
  (void) gd;
  return nullptr;
}

ModuleItem* Isolate::build(const IntegerDeclaration* id) {
  // Careful: We don't want what's on the rhs of the assignment, we want the
  // value of the identifier, which may have different sign/size.
  // Careful: We aren't allowed to have initial values for arrays
  auto res = new RegDeclaration(
    id->get_attrs()->accept(this),
    id->accept_id(this),
    true,
    new RangeExpression(32,0),
    id->get_id()->empty_dim() ? 
      new Number(Evaluate().get_value(id->get_id()), Number::HEX) :
      nullptr
  );
  return res;
}

ModuleItem* Isolate::build(const LocalparamDeclaration* ld) {
  // Careful: We don't want what's on the rhs of the assignment, we want the
  // value of the identifier, which may have different sign/size.
  auto res = new LocalparamDeclaration(
    ld->get_attrs()->accept(this),
    ld->get_signed(),
    ld->accept_dim(this),
    ld->accept_id(this),
    new Number(Evaluate().get_value(ld->get_id()), Number::HEX)
  );
  res->get_attrs()->push_back_as(new AttrSpec(
    new Identifier("__id"),
    Resolve().get_full_id(ld->get_id())
  ));
  return res;
}

ModuleItem* Isolate::build(const ParameterDeclaration* pd) {
  // Parameter declarations are downgraded to local params
  // Careful: We don't want what's on the rhs of the assignment, we want the
  // value of the identifier, which may have different sign/size.
  auto res = new LocalparamDeclaration(
    pd->get_attrs()->accept(this),
    pd->get_signed(),
    pd->accept_dim(this),
    pd->accept_id(this),
    new Number(Evaluate().get_value(pd->get_id()), Number::HEX)
  );
  res->get_attrs()->push_back_as(new AttrSpec(
    new Identifier("__id"),
    Resolve().get_full_id(pd->get_id())
  ));
  return res;
}

ModuleItem* Isolate::build(const RegDeclaration* rd) {
  // Careful: We don't want what's on the rhs of the assignment, we want the
  // value of the identifier, which may have different sign/size.
  // Careful: We aren't allowed to have initial values for arrays
  auto res = new RegDeclaration(
    rd->get_attrs()->accept(this),
    rd->accept_id(this),
    rd->get_signed(),
    rd->accept_dim(this),
    rd->get_id()->empty_dim() ? 
      new Number(Evaluate().get_value(rd->get_id()), Number::HEX) :
      nullptr
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
  auto res = new Identifier(new Id(ss.str()));
  id->accept_dim(this, res->back_inserter_dim());
  return res;
}

Identifier* Isolate::to_global_id(const Identifier* id) {
  stringstream ss;
  ss << "__x" << isolate(id);
  auto res =  new Identifier(new Id(ss.str()));
  id->accept_dim(this, res->back_inserter_dim());
  return res;
}

ModuleDeclaration* Isolate::get_shell() {
  ModuleInfo info(src_);

  auto res = new ModuleDeclaration(
    src_->get_attrs()->clone(),
    to_mangled_id(dynamic_cast<const ModuleInstantiation*>(src_->get_parent())->get_iid())
  );

  unordered_set<const Identifier*> ports;
  ports.insert(info.reads().begin(), info.reads().end());
  ports.insert(info.writes().begin(), info.writes().end());

  for (auto p : ports) {
    res->push_back_ports(new ArgAssign(
      nullptr,
      to_global_id(p)
    ));

    const auto r = info.is_read(p);
    const auto w = info.is_write(p);
    const auto width = Evaluate().get_width(p);

    // TODO: Is this logic correct? When should a global read/write be promoted
    // to a register and when should it remain a net?

    auto pd = new PortDeclaration(
      new Attributes(), 
      r && w ? PortDeclaration::INOUT : r ? PortDeclaration::INPUT : PortDeclaration::OUTPUT,
      (info.is_local(p) && dynamic_cast<const RegDeclaration*>(p->get_parent())) ? 
        (Declaration*) new RegDeclaration(
          new Attributes(),
          to_global_id(p),
          dynamic_cast<const RegDeclaration*>(p->get_parent())->get_signed(), 
          width == 1 ? nullptr : new RangeExpression(width),
          dynamic_cast<const RegDeclaration*>(p->get_parent())->clone_val()
        ) : 
        (Declaration*) new NetDeclaration(
          new Attributes(),
          NetDeclaration::WIRE,
          nullptr,
          to_global_id(p),
          dynamic_cast<const NetDeclaration*>(p->get_parent())->get_signed(),
          width == 1 ? nullptr : new RangeExpression(width)
        )
    );
    pd->get_attrs()->push_back_as(new AttrSpec(
      new Identifier("__id"), 
      Resolve().get_full_id(p)
    ));
    res->push_back_items(pd);
  }

  return res;
}

vector<ModuleItem*> Isolate::get_local_decls() {
  vector<ModuleItem*> res;
  for (auto l : ModuleInfo(src_).locals()) {
    if (ModuleInfo(src_).is_read(l) || ModuleInfo(src_).is_write(l)) {
      continue;
    }
    auto d = dynamic_cast<const Declaration*>(l->get_parent())->accept(this);
    if (d != nullptr) {
      res.push_back(d);
    }
  }
  return res;
}

void Isolate::replace(vector<ModuleItem*>& res, const ModuleInstantiation* mi) {
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
      nullptr,
      new VariableAssign(lhs, rhs)
    );
    res.push_back(ca);
  }
}

void Isolate::flatten(vector<ModuleItem*>& res, const CaseGenerateConstruct* cgc) {
  if (Elaborate().is_elaborated(cgc)) {
    auto elab = Elaborate().get_elaboration(cgc);
    flatten(res, elab);
  }
}

void Isolate::flatten(vector<ModuleItem*>& res, const IfGenerateConstruct* igc) {
  if (Elaborate().is_elaborated(igc)) {
    auto elab = Elaborate().get_elaboration(igc);
    flatten(res, elab);
  }
}

void Isolate::flatten(vector<ModuleItem*>& res, const LoopGenerateConstruct* lgc) {
  if (Elaborate().is_elaborated(lgc)) {
    auto elab = Elaborate().get_elaboration(lgc);
    for (auto gb : elab) {
      flatten(res, gb);
    }
  }
}

void Isolate::flatten(vector<ModuleItem*>& res, const GenerateBlock* gb) {
  auto temp = get_items(gb->begin_items(), gb->end_items(), false);
  res.insert(res.end(), temp.begin(), temp.end()); 
}

void Isolate::flatten(vector<ModuleItem*>& res, const GenerateRegion* gr) {
  auto temp = get_items(gr->begin_items(), gr->end_items(), false);
  res.insert(res.end(), temp.begin(), temp.end()); 
}

} // namespace cascade
