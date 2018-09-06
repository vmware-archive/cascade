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

#include "src/verilog/analyze/module_info.h"

#include "src/verilog/ast/ast.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/program/elaborate.h"
#include "src/verilog/program/inline.h"

using namespace std;

namespace cascade {

ModuleInfo::ModuleInfo(const ModuleDeclaration* md) : Visitor() {
  md_ = const_cast<ModuleDeclaration*>(md);
  lhs_ = false;
  refresh();
}

void ModuleInfo::invalidate() {
  if (md_->next_update_ == 0) {
    return;
  }
  md_->next_update_ = 0;
  md_->locals_.clear();
  md_->externals_.clear();
  md_->inputs_.clear();
  md_->outputs_.clear();
  md_->stateful_.clear();
  md_->reads_.clear();
  md_->writes_.clear();
  md_->named_params_.clear();
  md_->ordered_params_.clear();
  md_->named_ports_.clear();
  md_->ordered_ports_.clear();
  md_->connections_.clear();
  md_->children_.clear();
}

bool ModuleInfo::is_declaration() {
  return md_->get_parent() == nullptr;
}

bool ModuleInfo::is_instantiated() {
  return md_->get_parent() != nullptr;
}

const Identifier* ModuleInfo::id() {
  auto p = md_->get_parent();
  if (p == nullptr) {
    return nullptr;
  }
  auto mi = dynamic_cast<ModuleInstantiation*>(p);
  assert(mi != nullptr);
  return mi->get_iid();
}

bool ModuleInfo::is_local(const Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  return r == nullptr ? false : (md_->locals_.find(r) != md_->locals_.end());
}

bool ModuleInfo::is_input(const Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  return r == nullptr ? false : md_->inputs_.find(r) != md_->inputs_.end();
}

bool ModuleInfo::is_stateful(const Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  return r == nullptr ? false : md_->stateful_.find(r) != md_->stateful_.end();
}

bool ModuleInfo::is_output(const Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  return r == nullptr ? false : md_->outputs_.find(r) != md_->outputs_.end();
}

bool ModuleInfo::is_external(const Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  return r == nullptr ? false : md_->externals_.find(r) != md_->externals_.end();
}

bool ModuleInfo::is_read(const Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  return r == nullptr ? false : md_->reads_.find(r) != md_->reads_.end();
}

bool ModuleInfo::is_write(const Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  return r == nullptr ? false : md_->writes_.find(r) != md_->writes_.end();
}

bool ModuleInfo::is_child(const Identifier* id) {
  const auto r = Resolve().get_resolution(id);
  return r == nullptr ? false : md_->children_.find(r) != md_->children_.end();
}

const unordered_set<const Identifier*>& ModuleInfo::locals() {
  return md_->locals_;
}

const unordered_set<const Identifier*>& ModuleInfo::inputs() {
  return md_->inputs_;
}

const unordered_set<const Identifier*>& ModuleInfo::outputs() {
  return md_->outputs_;
}

const unordered_set<const Identifier*>& ModuleInfo::stateful() {
  return md_->stateful_;
}

const unordered_set<const Identifier*>& ModuleInfo::externals() {
  return md_->externals_;
}

const unordered_set<const Identifier*>& ModuleInfo::reads() {
  return md_->reads_;
}

const unordered_set<const Identifier*>& ModuleInfo::writes() {
  return md_->writes_;
}

const unordered_map<const Identifier*, const ModuleDeclaration*>& ModuleInfo::children() {
  return md_->children_;
}

const unordered_set<const Identifier*, HashId, EqId>& ModuleInfo::named_params() {
  return md_->named_params_;
}

const vector<const Identifier*>& ModuleInfo::ordered_params() {
  return md_->ordered_params_;
}

const unordered_set<const Identifier*, HashId, EqId>& ModuleInfo::named_ports() {
  return md_->named_ports_;
}

const vector<const Identifier*>& ModuleInfo::ordered_ports() {
  return md_->ordered_ports_;
}

const unordered_map<const Identifier*, unordered_map<const Identifier*, const Expression*>>& ModuleInfo::connections() {
  return md_->connections_;
}

void ModuleInfo::named_parent_conn(const ModuleInstantiation* mi, const PortDeclaration* pd) {
  for (auto p : *mi->get_ports()) {
    // This is a named connection, so explicit port should never be null.
    // Typechecking should enforce this.
    assert(!p->get_exp()->null());
    // Nothing to do for an empty named connection
    if (p->get_imp()->null()) {
      continue;
    }
    // Nothing to do if this isn't the right port
    const auto r = Resolve().get_resolution(p->get_exp()->get()); 
    if (r != pd->get_decl()->get_id()) {
      continue;
    }

    // Flag this variable as either a read or a write and return
    switch (pd->get_type()) {
      case PortDeclaration::INPUT:
        record_local_read(r);
        return;
      case PortDeclaration::OUTPUT:
        record_local_write(r);
        return;
      default:
        record_local_read(r);
        record_local_write(r);
        return;
    }
  }
}

void ModuleInfo::ordered_parent_conn(const ModuleInstantiation* mi, const PortDeclaration* pd, size_t idx) {
  // Do nothing if this port doesn't appear in mi's port list
  if (idx >= mi->get_ports()->size()) {
    return;
  }
  auto p = mi->get_ports()->get(idx);

  // This is an ordered connection, so explicit port should always be null.
  // Typechecking should enforce this.
  assert(p->get_exp()->null());
  // Nothing to do for an empty ordered connection
  if (p->get_imp()->null()) {
    return;
  }

  // Flag this variable as either a read or a write
  const auto r = pd->get_decl()->get_id();
  switch (pd->get_type()) {
    case PortDeclaration::INPUT:
      record_local_read(r);
      break;
    case PortDeclaration::OUTPUT:
      record_local_write(r);
      break;
    default:
      record_local_read(r);
      record_local_write(r);
      break;
  }
}

void ModuleInfo::named_child_conns(const ModuleInstantiation* mi) {
  unordered_map<const Identifier*, const Expression*> conn;
  for (auto p : *mi->get_ports()) {
    // This is a named connection, so explicit port should never be null.
    // Typechecking should enforce this.
    assert(!p->get_exp()->null());
    // Nothing to do for an empty named connection
    if (p->get_imp()->null()) {
      continue;
    }

    // Grab the declaration that this explicit port corresponds to
    const auto r = Resolve().get_resolution(p->get_exp()->get()); 
    assert(r != nullptr);
    // Connect the variable tot he expression in this module
    conn.insert(make_pair(r, p->get_imp()->get()));

    // Anything that appears in a module's port list must be declared as
    // a port. Typechecking should enforce this.
    auto pd = dynamic_cast<const PortDeclaration*>(r->get_parent()->get_parent());
    assert(pd != nullptr);

    switch (pd->get_type()) {
      case PortDeclaration::INPUT:
        record_external_write(r);
        break;
      case PortDeclaration::OUTPUT:
        record_external_read(r);
        break;
      default:
        record_external_read(r);
        record_external_write(r);
        break;
    }
  }
  md_->connections_.insert(make_pair(mi->get_iid(), conn));
}

void ModuleInfo::ordered_child_conns(const ModuleInstantiation* mi) {
  unordered_map<const Identifier*, const Expression*> conn;

  auto itr = Elaborate().get_elaboration(mi)->get_items()->begin();
  auto itre = Elaborate().get_elaboration(mi)->get_items()->end();

  for (size_t i = 0, ie = mi->get_ports()->size(); i < ie; ++i) {
    const auto p = mi->get_ports()->get(i);
    // This is an ordered connection, so explicit port should always be null.
    // Typechecking should enforce this.
    assert(p->get_exp()->null());

    // Track to the first port declaration. It's kind of ugly to have to iterate
    // over the entire text of this module every time we refresh, but it's the price
    // we pay for not having to rely on its module info.
    while (dynamic_cast<const PortDeclaration*>(*itr) == nullptr && itr != itre) {
      ++itr;
    }
    assert(itr != itre);
    // Anything that appears in a module's port list must be declared as
    // a port. Typechecking should enforce this.
    auto pd = dynamic_cast<const PortDeclaration*>(*itr++);
    assert(pd != nullptr);

    // Nothing to do for an empty ordered connection
    if (p->get_imp()->null()) {
      continue;
    }

    // Grab the declaration that this port corresponds to
    const auto r = pd->get_decl()->get_id();
    // Connect the variable to the expression in this module
    conn.insert(make_pair(r, p->get_imp()->get()));

    switch (pd->get_type()) {
      case PortDeclaration::INPUT:
        record_external_write(r);
        break;
      case PortDeclaration::OUTPUT:
        record_external_read(r);
        break;
      default:
        record_external_read(r);
        record_external_write(r);
        break;
    }
  }
  md_->connections_.insert(make_pair(mi->get_iid(), conn));
}

void ModuleInfo::record_local_read(const Identifier* id) {
  if (md_->reads_.find(id) == md_->reads_.end()) {
    md_->reads_.insert(id);
  }
}

void ModuleInfo::record_external_read(const Identifier* id) {
  if (md_->externals_.find(id) == md_->externals_.end()) {
    md_->externals_.insert(id);
  }
  if (md_->reads_.find(id) == md_->reads_.end()) {
    md_->reads_.insert(id);
  }
}

void ModuleInfo::record_local_write(const Identifier* id) {
  if (md_->writes_.find(id) == md_->writes_.end()) {
    md_->writes_.insert(id);
  }
}

void ModuleInfo::record_external_write(const Identifier* id) {
  if (md_->externals_.find(id) == md_->externals_.end()) {
    md_->externals_.insert(id);
  }
  if (md_->writes_.find(id) == md_->writes_.end()) {
    md_->writes_.insert(id);
  }
}

void ModuleInfo::record_external_use(const Identifier* id) {
  for (auto i = Resolve().dep_begin(id), ie = Resolve().dep_end(id); i != ie; ++i) {
    // Is this an identifier that resolves here?
    if (auto eid = dynamic_cast<const Identifier*>(*i)) {
      // Nothing to do if this variable appears in this module
      if (Resolve().get_parent(eid) == md_) {
        continue;
      }
      // Do nothing If this is a named variable connection
      if (eid->get_parent() != nullptr && dynamic_cast<const ArgAssign*>(eid->get_parent()->get_parent())) {
        continue;
      }
      // If it's on the lhs of an expression, it's a write, otherwise it's a read
      auto va = dynamic_cast<const VariableAssign*>(eid->get_parent());
      if (va != nullptr && va->get_lhs() == eid) {
        record_local_read(id);
      } else {
        record_local_write(id);
      }
    }
  }
}

void ModuleInfo::visit(const Attributes* as) {
  // Does nothing. There's nothing for us in here other than the opportunity to
  // blow a ton of time looking up variables that we can't resolve.
  (void) as;
  return;
}

void ModuleInfo::visit(const Identifier* i) {
  // Do nothing if this is a local or unresolvable variable
  const auto r = Resolve().get_resolution(i);
  if (r == nullptr || (md_->locals_.find(r) != md_->locals_.end())) {
    return;
  }
  // This variable must be external, record read/write
  if (lhs_) {
    record_external_write(r);
  } else {
    record_external_read(r);
  }
}

void ModuleInfo::visit(const CaseGenerateConstruct* cgc) {
  cgc->get_cond()->accept(this);
  if (Elaborate().is_elaborated(cgc)) {
    Elaborate().get_elaboration(cgc)->accept(this);
  }
}

void ModuleInfo::visit(const IfGenerateConstruct* igc) {
  igc->get_if()->accept(this);
  if (Elaborate().is_elaborated(igc)) {
    Elaborate().get_elaboration(igc)->accept(this);
  }
}

void ModuleInfo::visit(const LoopGenerateConstruct* lgc) {
  lgc->get_init()->accept(this);
  lgc->get_cond()->accept(this);
  lgc->get_update()->accept(this);
  if (Elaborate().is_elaborated(lgc)) {
    Elaborate().get_elaboration(lgc)->accept(this);
  }
}

void ModuleInfo::visit(const GenvarDeclaration* gd) {
  md_->locals_.insert(gd->get_id());   
  // Nothing external should reference this
}

void ModuleInfo::visit(const IntegerDeclaration* id) {
  md_->locals_.insert(id->get_id());   
  // Nothing external should reference this
}

void ModuleInfo::visit(const LocalparamDeclaration* ld) {
  md_->locals_.insert(ld->get_id());   
  // Nothing external should reference this
}

void ModuleInfo::visit(const NetDeclaration* nd) {
  md_->locals_.insert(nd->get_id());   
  record_external_use(nd->get_id());
}

void ModuleInfo::visit(const ParameterDeclaration* pd) {
  md_->locals_.insert(pd->get_id());   
  // Nothing external should reference this
  md_->named_params_.insert(pd->get_id());
  md_->ordered_params_.push_back(pd->get_id());
}

void ModuleInfo::visit(const RegDeclaration* rd) {
  md_->locals_.insert(rd->get_id());   
  record_external_use(rd->get_id());
}

void ModuleInfo::visit(const ModuleInstantiation* mi) {
  // This module has been inlined, continue descending through here rather than
  // examine its connections.
  if (Inline().is_inlined(mi)) {
    return Inline().get_source(mi)->accept(this);
  }

  // Descend on implicit ports. These are syntactically part of this module.
  for (auto p : *mi->get_params()) {
    p->get_imp()->accept(this);
  }
  for (auto p : *mi->get_ports()) {
    p->get_imp()->accept(this);
  }

  // Nothing else to do if this module wasn't instantiated.
  if (!Elaborate().is_elaborated(mi)) {
    return;
  }
  // Otherwise, descend on port bindings to establish connections and record
  // this child.
  if (mi->uses_named_ports()) {
    named_child_conns(mi);
  } else {
    ordered_child_conns(mi);
  }
  md_->children_.insert(make_pair(mi->get_iid(), Elaborate().get_elaboration(mi)));
}

void ModuleInfo::visit(const PortDeclaration* pd) {
  // Record input or output port
  switch(pd->get_type()) {
    case PortDeclaration::INPUT:
      md_->inputs_.insert(pd->get_decl()->get_id());
      break;
    case PortDeclaration::OUTPUT:
      md_->outputs_.insert(pd->get_decl()->get_id());
      break;
    default:
      md_->inputs_.insert(pd->get_decl()->get_id());
      md_->outputs_.insert(pd->get_decl()->get_id());
      break;
  }
  // Record port name and ordering information
  md_->named_ports_.insert(pd->get_decl()->get_id());
  md_->ordered_ports_.push_back(pd->get_decl()->get_id());
  
  // Descend on declaration
  pd->get_decl()->accept(this);

  // Nothing else to do if this is a declaration
  if (is_declaration()) {
    return;
  }
  // Otherwise, update read/write information for this connection
  auto mi = dynamic_cast<const ModuleInstantiation*>(md_->get_parent());
  assert(mi != nullptr);
  if (mi->uses_named_ports()) {
    named_parent_conn(mi, pd);
  } else {
    ordered_parent_conn(mi, pd, md_->ordered_ports_.size()-1);
  }
}

void ModuleInfo::visit(const NonblockingAssign* na) {
  na->get_assign()->accept(this);

  auto r = Resolve().get_resolution(na->get_assign()->get_lhs());
  assert(r != nullptr);

  if (md_->locals_.find(r) == md_->locals_.end()) {
    return;
  }
  if (md_->stateful_.find(r) != md_->stateful_.end()) {
    return;
  }
  md_->stateful_.insert(r);
}

void ModuleInfo::visit(const VariableAssign* va) {
  lhs_ = true;
  va->get_lhs()->accept(this);
  lhs_ = false;
  va->get_rhs()->accept(this);
}

void ModuleInfo::refresh() {
  for (auto e = md_->get_items()->size(); md_->next_update_ < e; ++md_->next_update_) {
    md_->get_items()->get(md_->next_update_)->accept(this);
  }
}

} // namespace cascade
