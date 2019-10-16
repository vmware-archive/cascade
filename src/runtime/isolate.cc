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

#include "runtime/isolate.h"

#include <cassert>
#include <map>
#include <sstream>
#include <string>
#include "common/bits.h"
#include "runtime/data_plane.h"
#include "verilog/analyze/evaluate.h"
#include "verilog/analyze/module_info.h"
#include "verilog/analyze/navigate.h"
#include "verilog/analyze/resolve.h"
#include "verilog/program/elaborate.h"

using namespace std;

namespace cascade {

ModuleDeclaration* Isolate::isolate(const ModuleDeclaration* src, int ignore) {
  src_ = src; 
  ignore_ = ignore;
  return src_->accept(this);
}

VId Isolate::isolate(const Identifier* id) {
  const auto* r = Resolve().get_resolution(id);
  assert(r != nullptr);

  auto itr = symbol_table_.find(r);
  if (itr == symbol_table_.end()) {
    itr = symbol_table_.insert(make_pair(r, symbol_table_.size()+1)).first;
  }
  return itr->second;
}

MId Isolate::isolate(const ModuleInstantiation* mi) {
  auto itr = module_table_.find(mi->get_iid());
  if (itr == module_table_.end()) {
    itr = module_table_.insert(make_pair(mi->get_iid(), module_table_.size())).first;
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
  auto* res = get_shell();
  // Generate local declarations. 
  auto ld = get_local_decls();
  res->push_back_items(ld.begin(), ld.end());
  // Transform the remainder of the code
  auto mis = get_items(md->begin_items(), md->end_items(), true);
  res->push_back_items(mis.begin(), mis.end());

  return res;
}

ModuleItem* Isolate::build(const InitialConstruct* ic) {
  return (ignore_ >= 0) ?
    nullptr :
    new InitialConstruct(ic->accept_attrs(this), ic->accept_stmt(this));
}

ModuleItem* Isolate::build(const GenvarDeclaration* gd) {
  // Does nothing. We delete these.
  (void) gd;
  return nullptr;
}

ModuleItem* Isolate::build(const LocalparamDeclaration* ld) {
  // Careful: We don't want what's on the rhs of the assignment, we want the
  // value of the identifier, which may have different sign/size.
  auto* res = new LocalparamDeclaration(
    ld->get_attrs()->accept(this),
    ld->accept_id(this),
    ld->get_type(),
    ld->accept_dim(this),
    new Number(Evaluate().get_value(ld->get_id()), Number::Format::HEX)
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
  auto* res = new LocalparamDeclaration(
    pd->get_attrs()->accept(this),
    pd->accept_id(this),
    pd->get_type(),
    pd->accept_dim(this),
    new Number(Evaluate().get_value(pd->get_id()), Number::Format::HEX)
  );
  res->get_attrs()->push_back_as(new AttrSpec(
    new Identifier("__id"),
    Resolve().get_full_id(pd->get_id())
  ));
  return res;
}

ModuleItem* Isolate::build(const PortDeclaration* pd) {
  // Does nothing. We emit declarations at the top level.
  (void) pd;
  return nullptr;
}

Statement* Isolate::build(const ParBlock* pb) {
  // Until the day when we add support for delay statements, sequential
  // execution is guaranteed to be a valid scheduling of a par block.
  auto* res =  new SeqBlock();
  pb->accept_stmts(this, res->back_inserter_stmts());
  return res;
}

Statement* Isolate::build(const SeqBlock* sb) {
  auto* res = new SeqBlock();
  sb->accept_stmts(this, res->back_inserter_stmts());
  return res;
}

Statement* Isolate::build(const DebugStatement* ds) {
  // Get the fully qualified name of this scope
  const auto* id = Navigate(ds).name();
  assert(id != nullptr);
  auto* s = Resolve().get_full_id(id);
  assert(s != nullptr);

  // Create a new debug statement
  auto* res = ds->clone();
  const auto action = Evaluate().get_value(ds->get_action()).to_uint();

  // For all variants, if no arg was provided, attach the scope name
  if (ds->is_null_arg()) {
    res->replace_arg(s);
  }
  // If this is a call to list or showvars with an arg, prepend the scope to it.
  else if ((action == 0) || (action == 3)) {
    for (auto i = res->get_arg()->begin_ids(), ie = res->get_arg()->end_ids(); i != ie; ++i) {
      s->push_back_ids((*i)->clone());
    } 
    res->replace_arg(s);
  }

  return res;
}

Identifier* Isolate::to_mangled_id(const ModuleInstantiation* mi) {
  stringstream ss;
  ss << "__M" << isolate(mi);
  return new Identifier(ss.str());
}

Identifier* Isolate::to_local_id(const Identifier* id) {
  stringstream ss;
  ss << "__l" << isolate(id);
  auto* res = new Identifier(new Id(ss.str()));
  id->accept_dim(this, res->back_inserter_dim());
  return res;
}

Identifier* Isolate::to_global_id(const Identifier* id) {
  stringstream ss;
  ss << "__x" << isolate(id);
  auto* res =  new Identifier(new Id(ss.str()));
  id->accept_dim(this, res->back_inserter_dim());
  return res;
}

ModuleDeclaration* Isolate::get_shell() {
  ModuleInfo info(src_);

  assert(src_->get_parent()->is(Node::Tag::module_instantiation));
  auto* res = new ModuleDeclaration(
    src_->get_attrs()->clone(),
    to_mangled_id(static_cast<const ModuleInstantiation*>(src_->get_parent()))
  );

  // Sort ports lexicographically by name so that we assign VIds
  // deterministically
  unordered_set<const Identifier*> ios;
  ios.insert(info.reads().begin(), info.reads().end());
  ios.insert(info.writes().begin(), info.writes().end());
  map<string, const Identifier*> ports;
  for (auto* i : ios) {
    ports.insert(make_pair(Resolve().get_readable_full_id(i), i));
  }

  // Generate port and localparam declarations
  for (auto& p : ports) {
    const auto read = info.is_read(p.second);
    const auto write = info.is_write(p.second);
    const auto width = Evaluate().get_width(p.second);

    // If this is an external variable which corresponds to a parameter, and is
    // read by this module, we can replace it with a localparam. If it's
    // written, we can ignore it entirely.
    const auto* r = Resolve().get_resolution(p.second); 
    assert(r != nullptr);
    assert(r->get_parent() != nullptr);
    assert(r->get_parent()->is_subclass_of(Node::Tag::declaration));
    const auto* decl = static_cast<const Declaration*>(r->get_parent());
    if (decl->is(Node::Tag::localparam_declaration) || decl->is(Node::Tag::parameter_declaration)) {
      if (write) {
        res->push_back_items(new LocalparamDeclaration(
          new Attributes(),
          to_global_id(p.second),        
          decl->get_type(),
          decl->clone_dim(),
          decl->is(Node::Tag::localparam_declaration) ? 
            static_cast<const LocalparamDeclaration*>(decl)->clone_val() :
            static_cast<const ParameterDeclaration*>(decl)->clone_val()
        ));
      }
      continue;
    }

    // Otherwise, turn this signal into a port
    res->push_back_ports(new ArgAssign(
      nullptr,
      to_global_id(p.second)
    ));

    // TODO(eschkufz) Is this logic correct? When should a global read/write be
    // promoted to a register and when should it remain a net? There could be a
    // funny interaction here if a dereference of an external register is
    // promoted to an input.

    const auto is_signed = p.second->get_parent()->is(Node::Tag::reg_declaration) ?
      static_cast<const RegDeclaration*>(p.second->get_parent())->get_type() :
      static_cast<const NetDeclaration*>(p.second->get_parent())->get_type();

    auto* pd = new PortDeclaration(
      new Attributes(), 
      (read && write) ? PortDeclaration::Type::INOUT : write ? PortDeclaration::Type::INPUT : PortDeclaration::Type::OUTPUT,
      (info.is_local(p.second) && p.second->get_parent()->is(Node::Tag::reg_declaration)) ? 
        static_cast<Declaration*>(new RegDeclaration(
          new Attributes(),
          to_global_id(p.second),
          is_signed,
          (width == 1) ? nullptr : new RangeExpression(width),
          static_cast<const RegDeclaration*>(p.second->get_parent())->clone_val()
        )) : 
        static_cast<Declaration*>(new NetDeclaration(
          new Attributes(),
          to_global_id(p.second),
          is_signed,
          (width == 1) ? nullptr : new RangeExpression(width)
        ))
    );
    pd->get_attrs()->push_back_as(new AttrSpec(
      new Identifier("__id"), 
      Resolve().get_full_id(p.second)
    ));
    res->push_back_items(pd);
  }

  return res;
}

vector<ModuleItem*> Isolate::get_local_decls() {
  // Sort variables lexicographically by name so that we assign VIds
  // deterministically
  map<string, const Identifier*> locals;
  for (auto* l : ModuleInfo(src_).locals()) {
    if (ModuleInfo(src_).is_read(l) || ModuleInfo(src_).is_write(l)) {
      continue;
    }
    locals.insert(make_pair(Resolve().get_readable_full_id(l), l));
  }

  vector<ModuleItem*> res;
  for (auto& l : locals) {
    assert(l.second->get_parent()->is_subclass_of(Node::Tag::declaration));
    auto* d = static_cast<const Declaration*>(l.second->get_parent())->accept(this);
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

  // Sort connections lexicographically by name so that we generate statements
  // deterministically.
  map<string, pair<const Identifier*, const Expression*>> sorted_conns;
  for (auto& c : itr->second) {
    sorted_conns.insert(make_pair(Resolve().get_readable_full_id(c.first), c));
  }

  for (const auto& sc : sorted_conns) {
    const auto& c = sc.second;
    auto* lhs = ModuleInfo(Resolve().get_origin(c.first)).is_input(c.first) ? 
        c.first->accept(this) : 
        static_cast<Identifier*>(c.second->accept(this));
    auto* rhs = ModuleInfo(Resolve().get_origin(c.first)).is_input(c.first) ? 
        static_cast<Expression*>(c.second->accept(this)) : 
        c.first->accept(this);
    auto* ca = new ContinuousAssign(lhs, rhs);
    res.push_back(ca);
  }
}

void Isolate::flatten(vector<ModuleItem*>& res, const CaseGenerateConstruct* cgc) {
  if (Elaborate().is_elaborated(cgc)) {
    auto* elab = Elaborate().get_elaboration(cgc);
    flatten(res, elab);
  }
}

void Isolate::flatten(vector<ModuleItem*>& res, const IfGenerateConstruct* igc) {
  if (Elaborate().is_elaborated(igc)) {
    auto* elab = Elaborate().get_elaboration(igc);
    flatten(res, elab);
  }
}

void Isolate::flatten(vector<ModuleItem*>& res, const LoopGenerateConstruct* lgc) {
  if (Elaborate().is_elaborated(lgc)) {
    auto elab = Elaborate().get_elaboration(lgc);
    for (auto* gb : elab) {
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
