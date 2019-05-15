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

#include "verilog/transform/block_flatten.h"

#include <vector>
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"

using namespace std;

namespace cascade {

BlockFlatten::BlockFlatten() : Editor() { }

void BlockFlatten::run(ModuleDeclaration* md) {
  for (auto i = md->begin_items(); i != md->end_items();) {
    switch ((*i)->get_tag()) {
      case Node::Tag::always_construct:
        (*i)->accept(this);
        if (can_delete(static_cast<AlwaysConstruct*>(*i)->get_stmt())) {
          i = md->purge_items(i);
          continue;
        }
        break;;
      case Node::Tag::initial_construct:
        (*i)->accept(this);
        if (can_delete(static_cast<InitialConstruct*>(*i)->get_stmt())) {
          i = md->purge_items(i);
          continue; 
        }
        break;

      default:
        break;
    }
    ++i;
  }
}

void BlockFlatten::edit(SeqBlock* sb) {
  vector<Statement*> ss;
  while (!sb->empty_stmts()) {
    // Recursively descend on next element
    sb->front_stmts()->accept(this);
    auto* s = sb->remove_front_stmts();

    // If we can delete this statement, get rid of it
    if (can_delete(s)) {
      Resolve().invalidate(s);
      delete s;
    }
    // If this is a sequential block, absorb its elements and delete it
    else if (s->is(Node::Tag::seq_block)) {
      auto* c = static_cast<SeqBlock*>(s);
      Resolve().invalidate(c);
      while (!c->empty_stmts()) {
        ss.push_back(c->remove_front_stmts());
      }
      delete c;
    } 
    // Otherwise, absorb this statement 
    else {
      ss.push_back(s);
    } 
  }
  for (auto* s : ss) {
    sb->push_back_stmts(s);
  }
}

void BlockFlatten::edit(CaseStatement* cs) {
  vector<CaseItem*> cis;
  while (!cs->empty_items()) {
    cs->front_items()->accept(this);
    auto* ci = cs->remove_front_items();

    // If we can delete this case, get rid of it
    if (can_delete(ci->get_stmt())) {
      Resolve().invalidate(ci);
      delete ci;
    }
    // If we can flatten it, turn it into a single statement
    else if (can_flatten(ci->get_stmt())) {
      auto* s = static_cast<SeqBlock*>(ci->get_stmt())->remove_front_stmts();
      Resolve().invalidate(ci->get_stmt());
      ci->replace_stmt(s);
      cis.push_back(ci);
    }
    // Otherwise just leave it as is
    else {
      cis.push_back(ci);
    }
  }
  for (auto* ci : cis) {
    cs->push_back_items(ci);
  }
}

void BlockFlatten::edit(ConditionalStatement* cs) {
  // DO NOT try to flatten the branches of a conditional. This could lead to an
  // unexpected rebinding of if/then/else precedence when this code is printed
  // and rescanned.
  cs->accept_then(this);
  cs->accept_else(this);
}

bool BlockFlatten::can_flatten(const Statement* s) const {
  if (!s->is(Node::Tag::seq_block)) {
    return false;
  }
  const auto* sb = static_cast<const SeqBlock*>(s);
  return sb->size_stmts() == 1;
}

bool BlockFlatten::can_delete(const Statement* s) const {
  switch (s->get_tag()) {
    case Node::Tag::seq_block:
      return static_cast<const SeqBlock*>(s)->empty_stmts();
    case Node::Tag::conditional_statement:
      return can_delete(static_cast<const ConditionalStatement*>(s)->get_then()) &&
             can_delete(static_cast<const ConditionalStatement*>(s)->get_else());
    case Node::Tag::case_statement:
      return static_cast<const CaseStatement*>(s)->empty_items();
    case Node::Tag::timing_control_statement:
      return can_delete(static_cast<const TimingControlStatement*>(s)->get_stmt());
    default:
      return false;
  }
}

} // namespace cascade

