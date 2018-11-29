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

#include "src/verilog/analyze/navigate.h"

#include <cassert>
#include "src/verilog/ast/ast.h"
#include "src/verilog/program/elaborate.h"
#include "src/verilog/program/inline.h"

using namespace std;

namespace cascade {

const Identifier* Navigate::name_iterator::operator*() const {
  return itr_->second.first;
}

Navigate::name_iterator& Navigate::name_iterator::operator++() {
  ++itr_;
  return *this;
}

bool Navigate::name_iterator::operator==(const name_iterator& rhs) const {
  return itr_ == rhs.itr_;
}

bool Navigate::name_iterator::operator!=(const name_iterator& rhs) const {
  return itr_ != rhs.itr_;
}

Navigate::name_iterator::name_iterator(Scope::NameMap::const_iterator itr) {
  itr_ = itr;
}

const Node* Navigate::child_iterator::operator*() const {
  return itr_->second;
}

Navigate::child_iterator& Navigate::child_iterator::operator++() {
  ++itr_;
  return *this;
}

bool Navigate::child_iterator::operator==(const child_iterator& rhs) const {
  return itr_ == rhs.itr_;
}

bool Navigate::child_iterator::operator!=(const child_iterator& rhs) const {
  return itr_ != rhs.itr_;
}

Navigate::child_iterator::child_iterator(Scope::ChildMap::const_iterator itr) {
  itr_ = itr;
}

Navigate::Navigate(const Node* n) : Visitor() { 
  where_ = const_cast<Node*>(n);

  // Corner Case: Did we attach to an arg assign? The only place we see these
  // is in explicit port and parameter assignments. These actually point DOWN
  // into the instantiation that they appear in.
  auto p = n->get_parent();
  if (auto aa = dynamic_cast<ArgAssign*>(p)) {
    if (n == aa->get_exp()) {
      auto mi = dynamic_cast<ModuleInstantiation*>(p->get_parent()->get_parent());
      assert(mi != nullptr);
      if (Elaborate().is_elaborated(mi)) {
        where_ = const_cast<ModuleDeclaration*>(Elaborate().get_elaboration(mi));
      }
    }
  }

  // For everything else, we can just follow parent pointers until we hit a scope boundary and refresh
  for (; !lost() && !boundary_check(); where_ = where_->get_parent());
  if (lost()) {
    return;
  }

  refresh();
}

void Navigate::invalidate() {
  assert(location_check());
  auto s = dynamic_cast<Scope*>(where_);
  if (s->next_supdate_ > 0) {
    s->next_supdate_ = 0;
    s->snames_.clear();
    s->schildren_.clear(); 
  }
}

void Navigate::up() {
  assert(location_check());

  // Follow parent pointers until we hit a scope boundary and refresh
  for (where_ = where_->get_parent(); !lost() && !boundary_check(); where_ = where_->get_parent());
  if (!lost()) {
    refresh();
  }
}

bool Navigate::down(const Id* id) {
  assert(location_check());

  // Fail if we can't find a child with this id
  const auto s = dynamic_cast<Scope*>(where_);
  const auto itr = s->schildren_.find(id);
  if (itr == s->schildren_.end()) {
    return false;
  }
  // Otherwise descend and refresh
  where_ = (Node*)itr->second;
  refresh();

  return true;
}

bool Navigate::lost() const {
  return where_ == nullptr;
}

bool Navigate::root() const {
  // General Case: There's nothing above the root
  const auto p = where_->get_parent();
  if (p == nullptr) {
    return true;
  }
  // Corner Case: If this is the elaborated root, we need to look above its instantiation
  if (p->get_parent() == nullptr) {
    return true;
  }
  // This isn't the root
  return false;
}

const Node* Navigate::where() const {
  return where_;
}

const Identifier* Navigate::name() const {
  assert(location_check());

  if (auto gb = dynamic_cast<GenerateBlock*>(where_)) {
    assert(!gb->get_id()->null());
    return gb->get_id()->get();
  }
  if (auto pb = dynamic_cast<ParBlock*>(where_)) {
    assert(!pb->get_id()->null());
    return pb->get_id()->get();
  }
  if (auto md = dynamic_cast<ModuleDeclaration*>(where_)) {
    auto p = md->get_parent();
    if (p == nullptr) {
      return nullptr;
    }
    auto mi = dynamic_cast<ModuleInstantiation*>(p);
    assert(mi != nullptr);
    return mi->get_iid();
  }
  if (auto pb = dynamic_cast<ParBlock*>(where_)) {
    assert(!pb->get_id()->null());
    return pb->get_id()->get();
  }
  if (auto sb = dynamic_cast<SeqBlock*>(where_)) {
    assert(!sb->get_id()->null());
    return sb->get_id()->get();
  }

  assert(false);
  return nullptr;
}

const Identifier* Navigate::find_name(const Id* id) {
  assert(location_check());

  // Fail if we can't find an id that matches this one. Otherwise return an
  // arbitrary result.
  const auto s = dynamic_cast<Scope*>(where_);
  const auto itr = s->snames_.find(id);
  return itr != s->snames_.end() ? itr->second.first : nullptr;
}

const Identifier* Navigate::find_duplicate_name(const Id* id) {
  assert(location_check());

  // Fail if there's at most one id that matches this one. Otherwise return an
  // element which is distinct from id. Because we keep two, we'll always be
  // able to find at least one.
  const auto s = dynamic_cast<Scope*>(where_);
  const auto itr = s->snames_.find(id);
  if (itr == s->snames_.end() || (itr->second.second == nullptr)) {
    return nullptr;
  }
  return itr->second.first->get_ids()->back() != id ? 
    itr->second.first :
    itr->second.second;
}

const Node* Navigate::find_child(const Id* id) {
  assert(location_check());
  const auto s = dynamic_cast<Scope*>(where_);
  const auto itr = s->schildren_.find(id);
  return itr != s->schildren_.end() ? itr->second : nullptr;
}

Navigate::name_iterator Navigate::name_begin() const {
  assert(location_check());
  const auto s = dynamic_cast<Scope*>(where_);
  return name_iterator(s->snames_.begin());
}

Navigate::name_iterator Navigate::name_end() const {
  assert(location_check());
  const auto s = dynamic_cast<Scope*>(where_);
  return name_iterator(s->snames_.end());
}

Navigate::child_iterator Navigate::child_begin() const {
  assert(location_check());
  const auto s = dynamic_cast<Scope*>(where_);
  return child_iterator(s->schildren_.begin());
}

Navigate::child_iterator Navigate::child_end() const {
  assert(location_check());
  const auto s = dynamic_cast<Scope*>(where_);
  return child_iterator(s->schildren_.end());
}

bool Navigate::boundary_check() const {
  if (auto gb = dynamic_cast<const GenerateBlock*>(where_)) {
    if (!gb->get_id()->null()) {
      return true; 
    }
  } else if (auto bs = dynamic_cast<const BlockStatement*>(where_)) {
    if (!bs->get_id()->null()) {
      return true; 
    }
  } else if (dynamic_cast<const ModuleDeclaration*>(where_)) {
    return true;
  }
  return false;
}

bool Navigate::location_check() const {
  if (lost()) {
    return false;
  }
  return dynamic_cast<Scope*>(where_);
}

void Navigate::cache_name(const Identifier* id) {
  const auto s = dynamic_cast<Scope*>(where_);
  assert(s != nullptr);

  const auto i = id->get_ids()->front();
  auto itr = s->snames_.find(i);
  if (itr == s->snames_.end()) {
    s->snames_.insert(make_pair(i, make_pair(id, nullptr)));
  } else {
    itr->second.second = id;
  }
}

void Navigate::visit(const GenerateBlock* gb) {
  // Only descend through this scope's root or unnamed blocks
  if (where_ == gb || gb->get_id()->null()) {
    gb->get_items()->accept(this);        
    return;
  } 
  // Otherwise we've hit a child
  const auto s = dynamic_cast<Scope*>(where_);
  assert(s != nullptr);
  s->schildren_.insert(make_pair(gb->get_id()->get()->get_ids()->front(), gb));
}

void Navigate::visit(const CaseGenerateConstruct* cgc) {
  if (Elaborate().is_elaborated(cgc)) {
    Elaborate().get_elaboration(cgc)->accept(this);
  }
}

void Navigate::visit(const IfGenerateConstruct* igc) {
  if (Elaborate().is_elaborated(igc)) {
    Elaborate().get_elaboration(igc)->accept(this);
  }
}

void Navigate::visit(const LoopGenerateConstruct* lgc) {
  if (Elaborate().is_elaborated(lgc)) {
    Elaborate().get_elaboration(lgc)->accept(this);
  }
}

void Navigate::visit(const GenvarDeclaration* gd) {
  cache_name(gd->get_id());
}

void Navigate::visit(const IntegerDeclaration* id) {
  cache_name(id->get_id());
}

void Navigate::visit(const LocalparamDeclaration* ld) {
  cache_name(ld->get_id());
}

void Navigate::visit(const NetDeclaration* nd) {
  cache_name(nd->get_id());
}

void Navigate::visit(const ParameterDeclaration* pd) {
  cache_name(pd->get_id());
}

void Navigate::visit(const RegDeclaration* rd) {
  cache_name(rd->get_id());
}

void Navigate::visit(const ModuleInstantiation* mi) {
  // Proceed straight down if this module was inlined; there's no scope here
  if (Inline().is_inlined(mi)) {
    Inline().get_source(mi)->accept(this);
    return;
  }
  // If it's been instantiated, cache the scope below it.
  if (Elaborate().is_elaborated(mi)) {
    const auto s = dynamic_cast<Scope*>(where_);
    assert(s != nullptr);
    const auto md = Elaborate().get_elaboration(mi);
    s->schildren_.insert(make_pair(mi->get_iid()->get_ids()->front(), md));
  } 
}

void Navigate::visit(const ParBlock* pb) {
  // Only descend through this scope's root or unnamed blocks
  if (where_ == pb || pb->get_id()->null()) {
    pb->get_decls()->accept(this);        
    pb->get_stmts()->accept(this);        
    return;
  } 
  // Otherwise we've hit a child
  const auto s = dynamic_cast<Scope*>(where_);
  assert(s != nullptr);
  s->schildren_.insert(make_pair(pb->get_id()->get()->get_ids()->front(), pb));
}

void Navigate::visit(const SeqBlock* sb) {
  // Only descend through this scope's root or unnamed blocks
  if (where_ == sb || sb->get_id()->null()) {
    sb->get_decls()->accept(this);        
    sb->get_stmts()->accept(this);        
    return;
  } 
  // Otherwise we've hit a child
  const auto s = dynamic_cast<Scope*>(where_);
  assert(s != nullptr);
  s->schildren_.insert(make_pair(sb->get_id()->get()->get_ids()->front(), sb));
}

void Navigate::refresh() {
  assert(location_check());
  auto s = dynamic_cast<Scope*>(where_);
  if (auto md = dynamic_cast<ModuleDeclaration*>(where_)) {
    for (auto e = md->get_items()->size(); s->next_supdate_ < e; ++s->next_supdate_) {
      md->get_items()->get(s->next_supdate_)->accept(this);
    }
  } else if (s->next_supdate_ == 0) {
    where_->accept(this);
    s->next_supdate_ = 1;
  }
}

} // namespace cascade
