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

#include "src/verilog/transform/loop_unroll.h"

#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/navigate.h"
#include "src/verilog/analyze/resolve.h"

using namespace std;

namespace cascade {

LoopUnroll::LoopUnroll() : Rewriter() { }

void LoopUnroll::run(ModuleDeclaration* md) {
  md->accept_items(this);

  // Invalidate cached state (we haven't added or deleted declarations, so
  // there's no need to invalidate the scope tree).
  Resolve().invalidate(md);
  ModuleInfo(md).invalidate();

  // Now that we're back on solid ground, reset the values of any variables in
  // this module which we may have updated while statically evaluating a loop.
  Reset r;
  md->accept_items(&r);
}

LoopUnroll::Unroll::Unroll() : Builder() { }

Statement* LoopUnroll::Unroll::build(const BlockingAssign* ba) {
  const auto& val = Evaluate().get_value(ba->get_assign()->get_rhs());
  Evaluate().assign_value(ba->get_assign()->get_lhs(), val);

  return ba->clone();
}

Statement* LoopUnroll::Unroll::build(const ForStatement* fs) {
  auto* sb = new SeqBlock();

  const auto& ival = Evaluate().get_value(fs->get_init()->get_rhs());
  Evaluate().assign_value(fs->get_init()->get_lhs(), ival);
  sb->push_back_stmts(new BlockingAssign(fs->get_init()->clone()));

  while (Evaluate().get_value(fs->get_cond()).to_bool()) {
    auto* s = fs->get_stmt()->accept(this);
    sb->push_back_stmts(s);

    const auto& uval = Evaluate().get_value(fs->get_update()->get_rhs());
    Evaluate().assign_value(fs->get_update()->get_lhs(), uval);
    sb->push_back_stmts(new BlockingAssign(fs->get_update()->clone()));
  }

  return sb;
}

Statement* LoopUnroll::Unroll::build(const RepeatStatement* rs) {
  const auto n = Evaluate().get_value(rs->get_cond()).to_int();
  auto* sb = new SeqBlock();
  for (size_t i = 0; i < n; ++i) {
    auto* s = rs->get_stmt()->accept(this);
    sb->push_back_stmts(s);
  }
  return sb;
}

LoopUnroll::Reset::Reset() : Visitor() { }

void LoopUnroll::Reset::visit(const IntegerDeclaration* id) {
  if (id->is_non_null_val()) {
    const auto& val = Evaluate().get_value(id->get_val());
    Evaluate().assign_value(id->get_id(), val);
  }
}

void LoopUnroll::Reset::visit(const RegDeclaration* rd) {
  if (rd->is_non_null_val()) {
    const auto& val = Evaluate().get_value(rd->get_val());
    Evaluate().assign_value(rd->get_id(), val);
  }
}

Statement* LoopUnroll::Unroll::build(const WhileStatement* ws) {
  auto* sb = new SeqBlock();
  while (Evaluate().get_value(ws->get_cond()).to_bool()) {
    auto* s = ws->get_stmt()->accept(this);
    sb->push_back_stmts(s);
  }
  return sb;
}

Statement* LoopUnroll::rewrite(ForStatement* fs) {
  Unroll u;
  return fs->accept(&u);
}

Statement* LoopUnroll::rewrite(RepeatStatement* rs) {
  Unroll u;
  return rs->accept(&u);
}

Statement* LoopUnroll::rewrite(WhileStatement* ws) {
  Unroll u;
  return ws->accept(&u);
}


} // namespace cascade
