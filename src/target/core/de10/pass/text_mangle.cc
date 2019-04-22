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

#include "src/target/core/de10/pass/text_mangle.h"

#include <sstream>
#include "src/target/core/de10/de10_logic.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/print/text/text_printer.h"

using namespace std;

namespace cascade {

TextMangle::TextMangle(const De10Logic* de) : Visitor() {
  de_ = de;
  within_task_ = false;
  io_idx_ = 0;
  task_idx_ = 0;
}

void TextMangle::replace(const Statement* s1, const Statement* s2) {
  auto itr = reps_.find(s1);
  assert(itr != reps_.end());
  reps_.insert(make_pair(s2, itr->second));
  reps_.erase(itr);
}

TextMangle::iterator TextMangle::find(const Statement* s) {
  return reps_.find(s);
}

TextMangle::iterator TextMangle::begin() {
  return reps_.begin();
}

TextMangle::iterator TextMangle::end() {
  return reps_.end();
}

void TextMangle::visit(const NonblockingAssign* na) {
  // Create empty blocks for true and false branches (we'll never populate the
  // false branch)
  auto* t = new SeqBlock();
  auto* f = new SeqBlock();

  // Look up the target of this assignment and the indices it spans in the
  // variable table
  const auto* lhs = na->get_assign()->get_lhs();
  const auto* r = Resolve().get_resolution(lhs);
  assert(r != nullptr);
  
  // Replace the original assignment with an assignment to a temporary variable
  auto* next = lhs->clone();
  next->purge_ids();
  next->push_back_ids(new Id(lhs->front_ids()->get_readable_sid() + "_next"));
  t->push_back_stmts(new NonblockingAssign(
    na->clone_ctrl(),
    new VariableAssign(
      next,
      na->get_assign()->get_rhs()->clone()
    )
  ));

  // Insert a new assignment to the next mask
  t->push_back_stmts(new NonblockingAssign(
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

  // Return a conditional statement in place of the original assignment
  reps_[na] = new SeqBlock(new ConditionalStatement(new Identifier("__live"), t, f));
}

void TextMangle::visit(const Identifier* id) {
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
    t_->push_back_stmts(new NonblockingAssign(
      new VariableAssign(
        new Identifier(new Id("__var"), new Number(Bits(32, titr->second.index()+i))),
        rhs
      )
    ));
  }
}

void TextMangle::visit(const DisplayStatement* ds) {
  begin_mangle_task();
  ds->accept_args(this);
  finish(ds);
}

void TextMangle::visit(const ErrorStatement* es) {
  begin_mangle_task();
  es->accept_args(this);
  finish(es);
}

void TextMangle::visit(const FinishStatement* fs) {
  begin_mangle_task();
  fs->accept_arg(this);
  finish(fs);
}

void TextMangle::visit(const GetStatement* gs) {
  (void) gs;
  begin_mangle_io();
  finish(gs);
}

void TextMangle::visit(const InfoStatement* is) {
  begin_mangle_task();
  is->accept_args(this);
  finish(is);
}

void TextMangle::visit(const PutStatement* ps) {
  (void) ps;
  begin_mangle_io();
  finish(ps);
}

void TextMangle::visit(const RestartStatement* rs) {
  (void) rs;
  begin_mangle_task();
  finish(rs);
}

void TextMangle::visit(const RetargetStatement* rs) {
  (void) rs;
  begin_mangle_task();
  finish(rs);
}

void TextMangle::visit(const SaveStatement* ss) {
  (void) ss;
  begin_mangle_task();
  finish(ss);
}

void TextMangle::visit(const SeekStatement* ss) {
  (void) ss;
  begin_mangle_io();
  finish(ss);
}

void TextMangle::visit(const WarningStatement* ws) {
  begin_mangle_task();
  ws->accept_args(this);
  finish(ws);
}

void TextMangle::visit(const WriteStatement* ws) {
  begin_mangle_task();
  ws->accept_args(this);
  finish(ws);
}

void TextMangle::begin_mangle_io() {
  within_task_ = true;
  t_ = new SeqBlock();
  t_->push_back_stmts(new NonblockingAssign(
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

void TextMangle::begin_mangle_task() {
  within_task_ = true;
  t_ = new SeqBlock();
  t_->push_back_stmts(new NonblockingAssign(
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

void TextMangle::finish(const SystemTaskEnableStatement* s) {
  within_task_ = false;
  reps_[s] = new SeqBlock(new ConditionalStatement(new Identifier("__live"), t_, new SeqBlock()));
}

Expression* TextMangle::get_table_range(const Identifier* r, const Identifier* i) {
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
