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

#include "src/verilog/analyze/resolve.h"

#include "src/verilog/analyze/indices.h"
#include "src/verilog/analyze/navigate.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/program/elaborate.h"
#include "src/verilog/program/inline.h"

using namespace std;

namespace cascade {

Resolve::Resolve() : Editor() { }

void Resolve::invalidate(Node* n) {
  n->accept(this);
}

const Identifier* Resolve::get_resolution(const Identifier* id) {
  // Fast Path: The result has been cached
  if (id->resolution_ != nullptr) {
    return id->resolution_;
  }
  // Slow Path: Perform resolution
  const auto r = resolution_impl(id);
  const_cast<Identifier*>(id)->resolution_ = r;

  // Nothing to do if we failed or if this is a self-pointer
  if (r == nullptr || r == id) {
    return r;
  }
  // Otherwise, add self-resolution for the target and update its dependents
  const_cast<Identifier*>(r)->resolution_ = r;
  for (Node* n = const_cast<Identifier*>(id); ; n = n->get_parent()) {
    if (auto e = dynamic_cast<Expression*>(n)) {
      if (find(r->dependents_.begin(), r->dependents_.end(), e) == r->dependents_.end()) {
        const_cast<Identifier*>(r)->dependents_.push_back(e);
      }
      if (find(e->dependencies_.begin(), e->dependencies_.end(), r) == e->dependencies_.end()) {
        e->dependencies_.push_back(const_cast<Identifier*>(r));
      }
    } else if (dynamic_cast<Combinator*>(n) == nullptr) {
      break;
    }
  }
  return r;
}

bool Resolve::is_slice(const Identifier* id) {
  const auto r = get_resolution(id);
  assert(r != nullptr);
  return (r == id) ? false : (id->get_dim()->size() > r->get_dim()->size());
}

Identifier* Resolve::get_full_id(const Identifier* id) {
  Navigate nav(id);
  auto fid = new Identifier(
    new Many<Id>(id->get_ids()->back()->clone()),
    id->get_dim()->clone()
  );
  while (!nav.lost() && nav.name() != nullptr) {
    fid->get_ids()->push_front(nav.name()->get_ids()->front()->clone());
    nav.up();
  }
  return fid;
}

const ModuleDeclaration* Resolve::get_parent(const Identifier* id) {
  for (const Node* n = id; n != nullptr; n = n->get_parent()) {
    if (auto md = dynamic_cast<const ModuleDeclaration*>(n)) {
      return md;
    }
  }
  return nullptr;
}

const ModuleDeclaration* Resolve::get_origin(const Identifier* id) {
  return get_parent(get_resolution(id));
}

Resolve::dep_iterator Resolve::dep_begin(const Identifier* id) {
  return id->dependents_.begin();
}

Resolve::dep_iterator Resolve::dep_end(const Identifier* id) {
  return id->dependents_.end();
}

const Identifier* Resolve::resolution_impl(const Identifier* id) {
  // Attach to the scope that encloses id
  Navigate nav(id);
  if (nav.lost()) {
    return nullptr;
  } 

  // Easy Case: Id has arity 1; seek up the hierarchy until we find id
  if (id->get_ids()->size() == 1) {
    while (!nav.lost()) {
      const auto r = nav.find_name(id->get_ids()->front());
      if (r != nullptr) {
        return r;
      }
      nav.up();
    }
    return nullptr;
  }

  // Hard Case (1/3): Seek up the hierarchy
  while (!nav.down(id->get_ids()->front()) && !nav.root()) { 
    nav.up();
  }
  if (nav.root() && (nav.name() == nullptr || !EqId()(nav.name()->get_ids()->front(), id->get_ids()->front()))) {
    return nullptr;
  }
  // Hard Case (2/3): Seek down the hierarchy
  for (size_t i = 1; i < id->get_ids()->size()-1; ++i) {
    if (!nav.down(id->get_ids()->get(i))) {
      return nullptr;
    }  
  }
  // Hard Case (3/3): It's either here or it isn't
  return nav.find_name(id->get_ids()->back());
}

void Resolve::edit(BinaryExpression* be) {
  Editor::edit(be);
  release(be);
}

void Resolve::edit(ConditionalExpression* ce) {
  Editor::edit(ce);
  release(ce);
}

void Resolve::edit(NestedExpression* ne) {
  Editor::edit(ne);
  release(ne);
}

void Resolve::edit(Concatenation* c) {
  Editor::edit(c);
  release(c);
}

void Resolve::edit(Identifier* id) {
  Editor::edit(id);
  release(id);
}

void Resolve::edit(MultipleConcatenation* mc) {
  Editor::edit(mc);
  release(mc);
}

void Resolve::edit(Number* n) {
  Editor::edit(n);
  release(n);
}

void Resolve::edit(String* s) {
  Editor::edit(s);
  release(s);
}

void Resolve::edit(RangeExpression* re) {
  Editor::edit(re);
  release(re);
}

void Resolve::edit(UnaryExpression* ue) {
  Editor::edit(ue);
  release(ue);
}

void Resolve::edit(CaseGenerateConstruct* cgc) {
  Editor::edit(cgc);
  if (Elaborate().is_elaborated(cgc)) {
    Elaborate().elaborate(cgc)->accept(this);
  }
}

void Resolve::edit(IfGenerateConstruct* igc) {
  Editor::edit(igc);
  if (Elaborate().is_elaborated(igc)) {
    Elaborate().elaborate(igc)->accept(this);
  }
}

void Resolve::edit(LoopGenerateConstruct* lgc) {
  Editor::edit(lgc);
  if (Elaborate().is_elaborated(lgc)) {
    Elaborate().elaborate(lgc)->accept(this);
  }
}

void Resolve::edit(ModuleInstantiation* mi) {
  Editor::edit(mi);
  if (Elaborate().is_elaborated(mi)) {
    Elaborate().elaborate(mi)->accept(this);
  }
  if (Inline().is_inlined(mi)) {
    // TODO: We don't have a non-const way of accessing inlined code
    assert(false);
  }
}

void Resolve::release(Expression* e) {
  for (auto d : e->dependencies_) {
    const auto itr = find(d->dependents_.begin(), d->dependents_.end(), e);
    assert(itr != d->dependents_.end());
    d->dependents_.erase(itr);
  }
  e->dependencies_.clear();
}

} // namespace cascade
