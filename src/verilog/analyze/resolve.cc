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

#include "src/verilog/analyze/resolve.h"

#include "src/verilog/analyze/indices.h"
#include "src/verilog/analyze/navigate.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/program/elaborate.h"
#include "src/verilog/program/inline.h"

using namespace std;

namespace cascade {

void Resolve::invalidate(const Node* n) {
  Invalidate inv;
  const_cast<Node*>(n)->accept(&inv);
}

const Identifier* Resolve::get_resolution(const Identifier* id) {
  // Fast Path: The result has been cached
  if (id->resolution_ != nullptr) {
    return id->resolution_;
  }
  // Slow Path: Perform resolution
  const auto r = cache_resolution(id);
  const_cast<Identifier*>(id)->resolution_ = r;
  // Nothing to do if we failed or if this is a self-pointer
  if (r == nullptr || r == id) {
    return r;
  }
  // Otherwise, add self-resolution for the target and update its dependents
  const_cast<Identifier*>(r)->resolution_ = r;
  return r;
}

Identifier* Resolve::get_full_id(const Identifier* id) {
  Navigate nav(id);
  auto fid = id->clone();
  fid->purge_ids();
  fid->push_back_ids(id->back_ids()->clone());
  while (!nav.lost() && nav.name() != nullptr) {
    fid->push_front_ids(nav.name()->front_ids()->clone());
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

bool Resolve::is_slice(const Identifier* id) {
  const auto r = get_resolution(id);
  assert(r != nullptr);
  return (r == id) ? false : (id->size_dim() > r->size_dim());
}

bool Resolve::is_scalar(const Identifier* id) {
  const auto r = get_resolution(id);
  assert(r != nullptr);
  return (id != r) || r->empty_dim();
}

bool Resolve::is_array(const Identifier* id) {
  return !is_scalar(id);
}

Resolve::use_iterator Resolve::use_begin(const Identifier* id) {
  const auto r = get_resolution(id);
  assert(r != nullptr);
  const auto d = dynamic_cast<const Declaration*>(r->get_parent());
  assert(d != nullptr);

  if (d->uses_ == nullptr) {
    cache_uses(d);
  }
  return d->uses_->begin();
}

Resolve::use_iterator Resolve::use_end(const Identifier* id) {
  const auto r = get_resolution(id);
  assert(r != nullptr);
  const auto d = dynamic_cast<const Declaration*>(r->get_parent());
  assert(d != nullptr);

  if (d->uses_ == nullptr) {
    cache_uses(d);
  }
  return d->uses_->end();
}

const Identifier* Resolve::cache_resolution(const Identifier* id) {
  // Attach to the scope that encloses id
  Navigate nav(id);
  if (nav.lost()) {
    return nullptr;
  } 

  // Easy Case: Id has arity 1; seek up the hierarchy until we find id
  if (id->size_ids() == 1) {
    while (!nav.lost()) {
      const auto r = nav.find_name(id->front_ids());
      if (r != nullptr) {
        return r;
      }
      nav.up();
    }
    return nullptr;
  }

  // Hard Case (1/3): Seek up the hierarchy
  while (!nav.down(id->front_ids()) && !nav.root()) { 
    nav.up();
  }
  if (nav.root() && (nav.name() == nullptr || !EqId()(nav.name()->front_ids(), id->front_ids()))) {
    return nullptr;
  }
  // Hard Case (2/3): Seek down the hierarchy
  for (size_t i = 1; i < id->size_ids()-1; ++i) {
    if (!nav.down(id->get_ids(i))) {
      return nullptr;
    }  
  }
  // Hard Case (3/3): It's either here or it isn't
  return nav.find_name(id->back_ids());
}

void Resolve::cache_uses(const Declaration* d) {
  // Navigate to the root of the module hierarchy
  Navigate nav(d);
  while (!nav.root()) {
    nav.up();
  }
  assert(!nav.lost());

  // Init and populate use sets 
  InitCacheUses icu;
  const_cast<Node*>(nav.where())->accept(&icu);
  CacheUses cu;
  const_cast<Node*>(nav.where())->accept(&cu);
}

void Resolve::InitCacheUses::edit(CaseGenerateConstruct* cgc) {
  Editor::edit(cgc);
  if (Elaborate().is_elaborated(cgc)) {
    Elaborate().get_elaboration(cgc)->accept(this);
  }
}

void Resolve::InitCacheUses::edit(IfGenerateConstruct* igc) {
  Editor::edit(igc);
  if (Elaborate().is_elaborated(igc)) {
    Elaborate().get_elaboration(igc)->accept(this);
  }
}

void Resolve::InitCacheUses::edit(LoopGenerateConstruct* lgc) {
  Editor::edit(lgc);
  if (Elaborate().is_elaborated(lgc)) {
    for (auto b : Elaborate().get_elaboration(lgc)) {
      b->accept(this);
    }
  }
}

void Resolve::InitCacheUses::edit(GenvarDeclaration* gd) {
  if (gd->uses_ == nullptr) {
    gd->uses_ = new Vector<const Expression*>();
  }
}

void Resolve::InitCacheUses::edit(IntegerDeclaration* id) {
  if (id->uses_ == nullptr) {
    id->uses_ = new Vector<const Expression*>();
  }
}

void Resolve::InitCacheUses::edit(LocalparamDeclaration* ld) {
  if (ld->uses_ == nullptr) {
    ld->uses_ = new Vector<const Expression*>();
  }
}

void Resolve::InitCacheUses::edit(NetDeclaration* nd) {
  if (nd->uses_ == nullptr) {
    nd->uses_ = new Vector<const Expression*>();
  }
}

void Resolve::InitCacheUses::edit(ParameterDeclaration* pd) {
  if (pd->uses_ == nullptr) {
    pd->uses_ = new Vector<const Expression*>();
  }
}

void Resolve::InitCacheUses::edit(RegDeclaration* rd) {
  if (rd->uses_ == nullptr) {
    rd->uses_ = new Vector<const Expression*>();
  }
}

void Resolve::InitCacheUses::edit(ModuleInstantiation* mi) {
  Editor::edit(mi);
  if (Elaborate().is_elaborated(mi)) {
    Elaborate().get_elaboration(mi)->accept(this);
  }
  if (Inline().is_inlined(mi)) {
    const_cast<IfGenerateConstruct*>(Inline().get_source(mi))->accept(this);
  }
}

void Resolve::CacheUses::edit(Attributes* as) {
  // Nothing to do. Don't descend past here.
  (void) as;
}

void Resolve::CacheUses::edit(Identifier* i) {
  Editor::edit(i);

  const auto r = Resolve().get_resolution(i);
  if (r == nullptr) {
    return;
  }
  const auto d = dynamic_cast<const Declaration*>(r->get_parent());
  assert(d != nullptr);
  assert(d->uses_ != nullptr);

  if (find(d->uses_->begin(), d->uses_->end(), i) != d->uses_->end()) {
    return;
  } 
  d->uses_->push_back(i);
  for (auto n = i->get_parent(); ; n = n->get_parent()) {
    if (auto e = dynamic_cast<const Expression*>(n)) {
      if (find(d->uses_->begin(), d->uses_->end(), e) == d->uses_->end()) {
        d->uses_->push_back(e);
      }
    } else {
      break;
    }
  }
}

void Resolve::CacheUses::edit(CaseGenerateConstruct* cgc) {
  Editor::edit(cgc);
  if (Elaborate().is_elaborated(cgc)) {
    Elaborate().get_elaboration(cgc)->accept(this);
  }
}

void Resolve::CacheUses::edit(IfGenerateConstruct* igc) {
  Editor::edit(igc);
  if (Elaborate().is_elaborated(igc)) {
    Elaborate().get_elaboration(igc)->accept(this);
  }
}

void Resolve::CacheUses::edit(LoopGenerateConstruct* lgc) {
  Editor::edit(lgc);
  if (Elaborate().is_elaborated(lgc)) {
    for (auto b : Elaborate().get_elaboration(lgc)) {
      b->accept(this);
    }
  }
}

void Resolve::CacheUses::edit(GenvarDeclaration* gd) {
  // Nothing to do. Don't descend past here.
  (void) gd;
}

void Resolve::CacheUses::edit(IntegerDeclaration* id) {
  id->accept_val(this);
}

void Resolve::CacheUses::edit(LocalparamDeclaration* ld) {
  ld->accept_dim(this);
  ld->accept_val(this);
}

void Resolve::CacheUses::edit(NetDeclaration* nd) {
  nd->accept_dim(this);
}

void Resolve::CacheUses::edit(ParameterDeclaration* pd) {
  pd->accept_dim(this);
  pd->accept_val(this);
}

void Resolve::CacheUses::edit(RegDeclaration* rd) {
  rd->accept_dim(this);
  rd->accept_val(this);
}

void Resolve::CacheUses::edit(ModuleInstantiation* mi) {
  Editor::edit(mi);
  if (Elaborate().is_elaborated(mi)) {
    Elaborate().get_elaboration(mi)->accept(this);
  }
  if (Inline().is_inlined(mi)) {
    const_cast<IfGenerateConstruct*>(Inline().get_source(mi))->accept(this);
  }
}

void Resolve::Invalidate::edit(Attributes* as) {
  // Don't descend past here
  (void) as;
}

void Resolve::Invalidate::edit(Identifier* id) {
  Editor::edit(id);
  id->resolution_ = nullptr;
}

void Resolve::Invalidate::edit(CaseGenerateConstruct* cgc) {
  Editor::edit(cgc);
  if (Elaborate().is_elaborated(cgc)) {
    Elaborate().get_elaboration(cgc)->accept(this);
  }
}

void Resolve::Invalidate::edit(IfGenerateConstruct* igc) {
  Editor::edit(igc);
  if (Elaborate().is_elaborated(igc)) {
    Elaborate().get_elaboration(igc)->accept(this);
  }
}

void Resolve::Invalidate::edit(LoopGenerateConstruct* lgc) {
  Editor::edit(lgc);
  if (Elaborate().is_elaborated(lgc)) {
    for (auto b : Elaborate().get_elaboration(lgc)) {
      b->accept(this);
    }
  }
}

void Resolve::Invalidate::edit(GenvarDeclaration* gd) {
  Editor::edit(gd);
  if (gd->uses_ != nullptr) {
    delete gd->uses_;
    gd->uses_ = nullptr;
  }
}

void Resolve::Invalidate::edit(IntegerDeclaration* id) {
  Editor::edit(id);
  if (id->uses_ != nullptr) {
    delete id->uses_;
    id->uses_ = nullptr;
  }
}

void Resolve::Invalidate::edit(LocalparamDeclaration* ld) {
  Editor::edit(ld);
  if (ld->uses_ != nullptr) {
    delete ld->uses_;
    ld->uses_ = nullptr;
  }
}

void Resolve::Invalidate::edit(NetDeclaration* nd) {
  Editor::edit(nd);
  if (nd->uses_ != nullptr) {
    delete nd->uses_;
    nd->uses_ = nullptr;
  }
}

void Resolve::Invalidate::edit(ParameterDeclaration* pd) {
  Editor::edit(pd);
  if (pd->uses_ != nullptr) {
    delete pd->uses_;
    pd->uses_ = nullptr;
  }
}

void Resolve::Invalidate::edit(RegDeclaration* rd) {
  Editor::edit(rd);
  if (rd->uses_ != nullptr) {
    delete rd->uses_;
    rd->uses_ = nullptr;
  }
}

void Resolve::Invalidate::edit(ModuleInstantiation* mi) {
  Editor::edit(mi);
  if (Elaborate().is_elaborated(mi)) {
    Elaborate().get_elaboration(mi)->accept(this);
  }
  if (Inline().is_inlined(mi)) {
    const_cast<IfGenerateConstruct*>(Inline().get_source(mi))->accept(this);
  }
}

} // namespace cascade
