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

#include "target/core/de10/pass/text_mangle.h"

#include <sstream>
#include "target/core/de10/de10_logic.h"
#include "verilog/analyze/module_info.h"
#include "verilog/ast/ast.h"
#include "verilog/print/text/text_printer.h"

using namespace std;

namespace cascade {

TextMangle::TextMangle(const ModuleDeclaration* md, const De10Logic* de) : Builder() {
  md_ = md;
  de_ = de;
}

Statement* TextMangle::get_io(size_t i) {
  assert(i < ios_.size());
  return ios_[i];
}

Statement* TextMangle::get_task(size_t i) {
  assert(i < tasks_.size());
  return tasks_[i];
}

Attributes* TextMangle::build(const Attributes* as) {
  (void) as;
  return new Attributes();
}

ModuleItem* TextMangle::build(const RegDeclaration* rd) {
  return ModuleInfo(md_).is_stateful(rd->get_id()) ? nullptr : rd->clone();
}

ModuleItem* TextMangle::build(const PortDeclaration* pd) {
  ModuleInfo info(md_);
  if (info.is_stateful(pd->get_decl()->get_id()) || info.is_input(pd->get_decl()->get_id())) {
    return nullptr;
  } else {
    return pd->get_decl()->clone();
  }
}

Expression* TextMangle::build(const FeofExpression* fe) {
  // This is a bit confusing: the de10 compiler has created an entry in the
  // variable table for the argument to this expression (like we do with
  // arguments to display statements). Prior to transfering control to the fpga
  // we'll place the result of this eof check into this location in hardware.
  const auto itr = de_->table_find(fe->get_fd());
  assert(itr != de_->table_end());
  return new Identifier(new Id("__var"), new Number(Bits(32, itr->second.index())));
}

Statement* TextMangle::build(const NonblockingAssign* na) {
  Mangle m(de_, false, ios_.size(), tasks_.size());
  na->accept(&m);
  return m.res_;
}

Statement* TextMangle::build(const DisplayStatement* ds) {
  return save_task(ds);
}

Statement* TextMangle::build(const ErrorStatement* es) {
  return save_task(es);
}

Statement* TextMangle::build(const FinishStatement* fs) {
  return save_task(fs);
}

Statement* TextMangle::build(const FseekStatement* fs) {
  return save_io(fs);
}

Statement* TextMangle::build(const GetStatement* gs) {
  return save_io(gs);
}

Statement* TextMangle::build(const InfoStatement* is) {
  return save_task(is);
}

Statement* TextMangle::build(const PutStatement* ps) {
  return save_io(ps);
}

Statement* TextMangle::build(const RestartStatement* rs) {
  return save_task(rs);
}

Statement* TextMangle::build(const RetargetStatement* rs) {
  return save_task(rs);
}

Statement* TextMangle::build(const SaveStatement* ss) {
  return save_task(ss);
}

Statement* TextMangle::build(const WarningStatement* ws) {
  return save_task(ws);
}

Statement* TextMangle::build(const WriteStatement* ws) {
  return save_task(ws);
}

Statement* TextMangle::save_io(const Statement* s) {
  Mangle m(de_, true, ios_.size(), tasks_.size());
  s->accept(&m);

  auto* res = new NonblockingAssign(new VariableAssign(
    new Identifier("__1"), 
    new Number(Bits(32, ios_.size()))
  ));
  ios_.push_back(m.res_);

  return res;
}

Statement* TextMangle::save_task(const Statement* s) {
  Mangle m(de_, true, ios_.size(), tasks_.size());
  s->accept(&m);

  auto* res = new NonblockingAssign(new VariableAssign(
    new Identifier("__2"), 
    new Number(Bits(32, tasks_.size()))
  ));
  tasks_.push_back(m.res_);

  return res;
}

TextMangle::Mangle::Mangle(const De10Logic* de, bool within_task, size_t io_idx, size_t task_idx) {
  de_ = de;
  within_task_ = within_task;
  io_idx_ = io_idx;
  task_idx_ = task_idx;
  res_ = nullptr;
}

void TextMangle::Mangle::visit(const NonblockingAssign* na) {
  res_ = new SeqBlock();

  // Look up the target of this assignment and the indices it spans in the
  // variable table
  const auto* lhs = na->get_assign()->get_lhs();
  const auto* r = Resolve().get_resolution(lhs);
  assert(r != nullptr);
  
  // Replace the original assignment with an assignment to a temporary variable
  auto* next = lhs->clone();
  next->purge_ids();
  next->push_back_ids(new Id(lhs->front_ids()->get_readable_sid() + "_next"));
  res_->push_back_stmts(new NonblockingAssign(
    na->clone_ctrl(),
    new VariableAssign(
      next,
      na->get_assign()->get_rhs()->clone()
    )
  ));

  // Insert a new assignment to the next mask
  res_->push_back_stmts(new NonblockingAssign(
    new VariableAssign(
      new Identifier(
        new Id("__next_update_mask"),
        get_table_range(r, lhs)
      ),
      new UnaryExpression(
        UnaryExpression::Op::TILDE,
        new Identifier(
          new Id("__next_update_mask"),
          get_table_range(r, lhs)
        )
      )
    )
  ));
}

void TextMangle::Mangle::visit(const Identifier* id) {
  if (!within_task_) {
    return;
  }

  const auto titr = de_->table_find(id);
  assert(titr != de_->table_end());

  // This is a bit nasty. The amount of space we set aside for this argument in
  // the variable table may exceed its actual bit-width. This is because the
  // width of the argument may have been implicitly extended if it's part of an
  // expression. 
  const auto* r = Resolve().get_resolution(id);
  assert(r != nullptr);
  const auto w = Evaluate().get_width(r);

  for (size_t i = 0; i < titr->second.entry_size(); ++i) {
    const auto upper = min(32*(i+1),w)-1;
    const auto lower = 32*i;

    // Create a sign extension mask: all zeros for unsigned values, 32 copies
    // of id's highest order bit for signed values.
    Expression* sext = nullptr;
    if (Evaluate().get_signed(id)) {
      sext = new MultipleConcatenation(
        new Number("32"),
        new Concatenation((w == 1) ?
          new Identifier(id->front_ids()->clone()) :
          new Identifier(id->front_ids()->clone(), new Number(Bits(32, w-1)))
        )
      );
    } else {
      sext = new Number(Bits(32, 0), Number::Format::HEX);
    }

    // Concatenate the rhs with the sign extension bits
    auto* lsbs = new Identifier(id->front_ids()->clone());
    id->clone_dim(lsbs->back_inserter_dim());
    if (lsbs->size_dim() > r->size_dim()) {
      lsbs->purge_to_dim(r->size_dim());
    }
    if (upper == lower) {
      lsbs->push_back_dim(new Number(Bits(32, upper)));
    } else if (upper > lower) {
      lsbs->push_back_dim(new RangeExpression(upper+1, lower));
    } 
    auto* rhs = new Concatenation(sext);
    rhs->push_back_exprs(lsbs);

    // Attach the concatenation to an assignment, we'll always have enough bits now
    res_->push_back_stmts(new NonblockingAssign(
      new VariableAssign(
        new Identifier(new Id("__var"), new Number(Bits(32, titr->second.index()+i))),
        rhs
      )
    ));
  }
}

void TextMangle::Mangle::visit(const DisplayStatement* ds) {
  begin_mangle_task();
  ds->accept_args(this);
}

void TextMangle::Mangle::visit(const ErrorStatement* es) {
  begin_mangle_task();
  es->accept_args(this);
}

void TextMangle::Mangle::visit(const FinishStatement* fs) {
  begin_mangle_task();
  fs->accept_arg(this);
}

void TextMangle::Mangle::visit(const FseekStatement* fs) {
  (void) fs;
  begin_mangle_io();
}

void TextMangle::Mangle::visit(const GetStatement* gs) {
  (void) gs;
  begin_mangle_io();
}

void TextMangle::Mangle::visit(const InfoStatement* is) {
  begin_mangle_task();
  is->accept_args(this);
}

void TextMangle::Mangle::visit(const PutStatement* ps) {
  (void) ps;
  begin_mangle_io();
}

void TextMangle::Mangle::visit(const RestartStatement* rs) {
  (void) rs;
  begin_mangle_task();
}

void TextMangle::Mangle::visit(const RetargetStatement* rs) {
  (void) rs;
  begin_mangle_task();
}

void TextMangle::Mangle::visit(const SaveStatement* ss) {
  (void) ss;
  begin_mangle_task();
}

void TextMangle::Mangle::visit(const WarningStatement* ws) {
  begin_mangle_task();
  ws->accept_args(this);
}

void TextMangle::Mangle::visit(const WriteStatement* ws) {
  begin_mangle_task();
  ws->accept_args(this);
}

void TextMangle::Mangle::begin_mangle_io() {
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
}

void TextMangle::Mangle::begin_mangle_task() {
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
}

Expression* TextMangle::Mangle::get_table_range(const Identifier* r, const Identifier* i) {
  // Look up r in the variable table
  const auto titr = de_->table_find(r);
  assert(titr != de_->table_end());
  assert(titr->second.materialized());

  // Start with an expression for where this variable begins in the variable table
  Expression* idx = new Number(Bits(32, titr->second.index()));

  // Now iterate over the arity of r and compute a symbolic expression 
  auto mul = titr->second.elements();
  auto iitr = i->begin_dim();
  for (auto a : titr->second.arity()) {
    mul /= a;
    idx = new BinaryExpression(
      idx,
      BinaryExpression::Op::PLUS,
      new BinaryExpression(
        (*iitr++)->clone(),
        BinaryExpression::Op::TIMES,
        new Number(Bits(32, mul*titr->second.element_size()))
      )
    );
  }
  return new RangeExpression(idx, RangeExpression::Type::PLUS, new Number(Bits(32, titr->second.element_size())));
}

} // namespace cascade
