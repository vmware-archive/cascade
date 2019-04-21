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

#include "src/target/core/de10/task_mangle.h"

#include "src/verilog/ast/ast.h"

namespace cascade {

TaskMangle::TaskMangle() : Rewriter() { }

ModuleDeclaration* TaskMangle::run(ModuleDeclaration* md) {
  io_idx_ = 0;
  task_idx_ = 0;
  return md->accept(this);
}

ModuleDeclaration* TaskMangle::rewrite(ModuleDeclaration* md) {
  Rewriter::rewrite(md);

  md->push_front_items(new RegDeclaration(
    new Attributes(),
    new Identifier("__live"),
    false,
    nullptr,
    new Number(Bits(false))
  ));
  md->push_front_items(new RegDeclaration(
    new Attributes(),
    new Identifier("__next_io_mask"),
    false,
    new RangeExpression(32, 0),
    new Number(Bits(32, 0))
  ));
  md->push_front_items(new RegDeclaration(
    new Attributes(),
    new Identifier("__io_mask"),
    false,
    new RangeExpression(32, 0),
    new Number(Bits(32, 0))
  ));
  md->push_front_items(new RegDeclaration(
    new Attributes(),
    new Identifier("__next_task_mask"),
    false,
    new RangeExpression(32, 0),
    new Number(Bits(32, 0))
  ));
  md->push_front_items(new RegDeclaration(
    new Attributes(),
    new Identifier("__task_mask"),
    false,
    new RangeExpression(32, 0),
    new Number(Bits(32, 0))
  ));

  return md;
}

Expression* TaskMangle::rewrite(EofExpression* ee) {
  (void) ee;
  return new Identifier("__eof");
}

Statement* TaskMangle::rewrite(DisplayStatement* ds) {
  begin_mangle_task();
  ds->accept_args(this);
  return finish();
}

Statement* TaskMangle::rewrite(ErrorStatement* es) {
  begin_mangle_task();
  es->accept_args(this);
  return finish();
}

Statement* TaskMangle::rewrite(FinishStatement* fs) {
  begin_mangle_task();
  fs->accept_arg(this);
  return finish();
}

Statement* TaskMangle::rewrite(GetStatement* gs) {
  (void) gs;
  begin_mangle_io();
  return finish();
}

Statement* TaskMangle::rewrite(InfoStatement* is) {
  begin_mangle_task();
  is->accept_args(this);
  return finish();
}

Statement* TaskMangle::rewrite(PutStatement* ps) {
  (void) ps;
  begin_mangle_io();
  return finish();
}

Statement* TaskMangle::rewrite(RestartStatement* rs) {
  (void) rs;
  begin_mangle_task();
  return finish();
}

Statement* TaskMangle::rewrite(RetargetStatement* rs) {
  (void) rs;
  begin_mangle_task();
  return finish();
}

Statement* TaskMangle::rewrite(SaveStatement* ss) { 
  (void) ss;
  begin_mangle_task();
  return finish();
}

Statement* TaskMangle::rewrite(SeekStatement* ss) {
  (void) ss;
  begin_mangle_io();
  return finish();
}

Statement* TaskMangle::rewrite(WarningStatement* ws) {
  begin_mangle_task();
  ws->accept_args(this);
  return finish();
}

Statement* TaskMangle::rewrite(WriteStatement* ws) {
  begin_mangle_task();
  ws->accept_args(this);
  return finish();
}

void TaskMangle::begin_mangle_io() {
  res_ = new SeqBlock();
  res_->push_back_stmts(new NonblockingAssign(
    new VariableAssign(
      new Identifier(
        new Id("__next_io_mask"),
        new Number(Bits(32, io_idx_))
      ),
      new UnaryExpression(
        UnaryExpression::Op::TILDE,
        new Identifier(
          new Id("__next_io_mask"),
          new Number(Bits(32, io_idx_))
        )
      )
    )    
  ));
  ++io_idx_;
}

void TaskMangle::begin_mangle_task() {
  res_ = new SeqBlock();
  res_->push_back_stmts(new NonblockingAssign(
    new VariableAssign(
      new Identifier(
        new Id("__next_task_mask"),
        new Number(Bits(32, task_idx_))
      ),
      new UnaryExpression(
        UnaryExpression::Op::TILDE,
        new Identifier(
          new Id("__next_task_mask"),
          new Number(Bits(32, task_idx_))
        )
      )
    )    
  ));
  ++task_idx_;
}

Statement* TaskMangle::finish() {
  return new ConditionalStatement(new Identifier("__live"), res_, new SeqBlock());
}

} // namespace cascade
