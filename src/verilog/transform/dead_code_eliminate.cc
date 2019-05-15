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

#include "verilog/transform/dead_code_eliminate.h"

#include "verilog/ast/ast.h"
#include "verilog/analyze/module_info.h"
#include "verilog/analyze/navigate.h"
#include "verilog/analyze/resolve.h"

namespace cascade {

DeadCodeEliminate::DeadCodeEliminate() : Editor() { }

void DeadCodeEliminate::run(ModuleDeclaration* md) {
  Index idx(this);
  md->accept(&idx);
  md->accept(this);
  ModuleInfo(md).invalidate();
}

DeadCodeEliminate::Index::Index(DeadCodeEliminate* dce) {
  dce_ = dce;
}

void DeadCodeEliminate::Index::visit(const Attributes* a) {
  // Does nothing
  (void) a;
}

void DeadCodeEliminate::Index::visit(const Identifier* i) {
  Visitor::visit(i);
  const auto* r = Resolve().get_resolution(i);
  if (r != nullptr) {
    dce_->use_.insert(r);
  }
}

void DeadCodeEliminate::Index::visit(const IntegerDeclaration* id) {
  id->accept_val(this);
}

void DeadCodeEliminate::Index::visit(const LocalparamDeclaration* ld) {
  ld->accept_dim(this);
  ld->accept_val(this);
}

void DeadCodeEliminate::Index::visit(const NetDeclaration* nd) {
  nd->accept_dim(this);
}

void DeadCodeEliminate::Index::visit(const ParameterDeclaration* pd) {
  pd->accept_dim(this);
  pd->accept_val(this);
}

void DeadCodeEliminate::Index::visit(const RegDeclaration* rd) {
  rd->accept_dim(this);
  rd->accept_val(this);
}

void DeadCodeEliminate::edit(ModuleDeclaration* md) {
  auto dead = false;
  for (auto i = md->begin_items(); i != md->end_items(); ) {
    if ((*i)->is_subclass_of(Node::Tag::declaration)) {
      auto* d = static_cast<Declaration*>(*i);
      const auto is_port = d->get_parent()->is(Node::Tag::port_declaration);
      const auto is_dead = use_.find(d->get_id()) == use_.end();
      if (!is_port && is_dead) {
        dead = true;
        i = md->purge_items(i);
        continue;
      }
    }
    ++i;
  }
  if (dead) {
    Navigate(md).invalidate();
  }
}

void DeadCodeEliminate::edit(ParBlock* pb) {
  auto dead = false;
  for (auto i = pb->begin_decls(); i != pb->end_decls(); ) {
    const auto is_dead = use_.find((*i)->get_id()) == use_.end();
    if (is_dead) {
      dead = true;
      i = pb->purge_decls(i);
    } else {
      ++i;
    }
  }
  if (dead) {
    Navigate(pb).invalidate();
  }
}

void DeadCodeEliminate::edit(SeqBlock* sb) {
  auto dead = false;
  for (auto i = sb->begin_decls(); i != sb->end_decls(); ) {
    const auto is_dead = use_.find((*i)->get_id()) == use_.end();
    if (is_dead) {
      dead = true;
      i = sb->purge_decls(i);
    } else {
      ++i;
    }
  }
  if (dead) {
    Navigate(sb).invalidate();
  }
}

} // namespace cascade
