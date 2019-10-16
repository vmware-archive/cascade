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

#include "target/core/de10/pass/machinify.h"

#include "verilog/ast/ast.h"

using namespace std;

namespace cascade {

Machinify::Generate::Generate(size_t idx) : Visitor() { 
  idx_ = idx;
}

const ConditionalStatement* Machinify::Generate::text() const {
  return text_;
}

size_t Machinify::Generate::name() const {
  return idx_;
}

size_t Machinify::Generate::final_state() const {
  return current().first;
}

Machinify::Generate::task_iterator Machinify::Generate::task_begin() const {
  return task_states_.begin();
}

Machinify::Generate::task_iterator Machinify::Generate::task_end() const {
  return task_states_.end();
}

void Machinify::Generate::run(const EventControl* ec, const Statement* s) {
  // Create a state machine with a single state
  machine_ = new CaseStatement(CaseStatement::Type::CASE, new Identifier(new Id("__state"), new Number(Bits(32, idx_))));

  // Populate the state machine
  next_state();
  s->accept(this);

  // Append a done state. If the last state in the machine is empty, we can use
  // that one. Otherwise, we'll create one last transition to an empty state.
  auto c = current();
  if (!c.second->empty_stmts()) {
    if (task_states_.empty() || (task_states_.back() != c.first)) {
      c.second->push_back_stmts(new NonblockingAssign(new Identifier(new Id("__task_id"), new Number(Bits(32, idx_))), new Number(Bits(32, 0))));
    }
    transition(c.second, c.first+1);
    next_state();
    c = current();
  }
  assert(c.second->get_parent()->is(Node::Tag::case_item));
  auto* ci = static_cast<CaseItem*>(c.second->get_parent());
  ci->purge_exprs();
  c.second->push_back_stmts(new NonblockingAssign(new Identifier(new Id("__task_id"), new Number(Bits(32, idx_))), new Number(Bits(32, 0))));
  c.second->push_back_stmts(new NonblockingAssign(new Identifier(new Id("__state"), new Number(Bits(32, idx_))), new Number(Bits(32, final_state()))));

  // Tie everything together into a conditional statement
  auto i = ec->begin_events();
  Expression* guard = to_guard(*i++);
  for (auto ie = ec->end_events(); i != ie; ++i) {
    guard = new BinaryExpression(to_guard(*i), BinaryExpression::Op::PPIPE, guard);
  }
  text_ = new ConditionalStatement(
    new Identifier("__continue"), 
    machine_,
    new SeqBlock(new NonblockingAssign(
      new Identifier(new Id("__state"), new Number(Bits(32, idx_))),
      new ConditionalExpression(
        new Identifier("__reset"),
        new Number(Bits(32, final_state())),
        new ConditionalExpression(guard, new Number(Bits(false)), new Identifier(new Id("__state"), new Number(Bits(32, idx_))))
      )
    ))
  );
}

void Machinify::Generate::visit(const BlockingAssign* ba) {
  append(ba);
}

void Machinify::Generate::visit(const NonblockingAssign* na) {
  // Check whether this is a task
  const auto is_task = na->get_lhs()->eq("__task_id");

  // If it's not, we can append it and move on. Otherwise, we need to record
  // the state that it appears in and mangle it a bit.
  if (!is_task) {
    append(na);
  } else {
    auto* c = na->clone();
    c->get_lhs()->push_front_dim(new Number(Bits(32, idx_)));
    append(c);
    delete c;
    task_states_.push_back(current().first);
  }

  // NOTE: We have the invariant that our code doesn't have any nested seq
  // blocks (which we didn't introduce ourselves, and by construction won't
  // have any tasks inside them).  
  
  // If this is the last statement in a seq block, it's already sitting at a
  // state boundary, and there's no need to introduce another break.
  const auto* p = na->get_parent();
  if (p->is(Node::Tag::seq_block)) {
    const auto* sb = static_cast<const SeqBlock*>(p);
    if (sb->back_stmts() == na) {
      return;
    }
  }
  // Otherwise, if this is a task, we'll need to break for a state transition.
  if (is_task) {
    transition(current().first+1);
    next_state();
  }
}

void Machinify::Generate::visit(const SeqBlock* sb) {
  sb->accept_stmts(this);
}

void Machinify::Generate::visit(const CaseStatement* cs) {
  // TODO(eschkufz) There are similar optimizations to the ones in
  // ConditionalStatement that can still be made here.

  if (!TaskCheck().run(cs)) {
    append(cs);
    return;
  } 

  const auto begin = current();

  vector<pair<size_t, SeqBlock*>> begins;
  vector<pair<size_t, SeqBlock*>> ends;
  for (auto i = cs->begin_items(), ie = cs->end_items(); i != ie; ++i) {
    next_state();
    begins.push_back(current());
    (*i)->accept_stmt(this);
    ends.push_back(current());
  }

  auto* branch = new CaseStatement(cs->get_type(), cs->get_cond()->clone());
  size_t idx = 0;
  for (auto i = cs->begin_items(), ie = cs->end_items(); i != ie; ++i) {
    branch->push_back_items(new CaseItem(
      new NonblockingAssign(
        new Identifier(new Id("__state"), new Number(Bits(32, idx_))),
        new Number(Bits(32, begins[idx++].first))
      )
    ));
    for (auto j = (*i)->begin_exprs(), je = (*i)->end_exprs(); j != je; ++j) {
      branch->back_items()->push_back_exprs((*j)->clone());
    }
  }
  append(begin.second, branch);
  
  next_state();
  for (auto& e : ends) {
    transition(e.second, current().first);
  }
}

void Machinify::Generate::visit(const ConditionalStatement* cs) {
  // No need to split a conditional statement that doesn't have any io
  if (!TaskCheck().run(cs)) {
    append(cs);
    return;
  }

  // Check whether this conditional has an empty else branch
  const auto empty_else = 
    cs->get_else()->is(Node::Tag::seq_block) &&
    static_cast<const SeqBlock*>(cs->get_else())->empty_stmts();
  // Check whether this is the last statement in a seq block
  const auto last_stmt = 
    cs->get_parent()->is(Node::Tag::seq_block) &&
    static_cast<const SeqBlock*>(cs->get_parent())->back_stmts() == cs;

  // Record the current state
  const auto begin = current();

  // We definitely need a new state for the true branch
  next_state();
  const auto then_begin = current();
  cs->get_then()->accept(this);
  const auto then_end = current();

  // We only need a new state for the else branch if it's non-empty.
  if (!empty_else) {
    next_state();
  }
  const auto else_begin = current();
  cs->get_else()->accept(this);
  const auto else_end = current();

  // And if this ISNT the last statement in a seq block or we have a non-empty
  // else, we need a phi node to join the two. 
  const auto phi_node = !empty_else || !last_stmt;
  if (phi_node) {
    next_state();
  }
  
  // And now we need transitions between the branches. The true branch always
  // goes to tbe beginning of the then state, and the else branch either goes
  // to the beginning of the else state or one past the end of the then state.
  auto* branch = new ConditionalStatement(
    cs->get_if()->clone(),
    new NonblockingAssign(
      new Identifier(new Id("__state"), new Number(Bits(32, idx_))),
      new Number(Bits(32, then_begin.first))
    ),
    new NonblockingAssign(
      new Identifier(new Id("__state"), new Number(Bits(32, idx_))),
      new Number(Bits(32, !empty_else ? else_begin.first : (then_end.first + 1)))
    )
  );
  append(begin.second, branch);

  // If we emitted a phi node, the then branch goes there (to the current state).
  // And if the else branch was non-empty, it goes there as well.
  if (phi_node) {
    transition(then_end.second, current().first);
    if (!empty_else) {
      transition(else_end.second, current().first);
    }
  }
}

pair<size_t, SeqBlock*> Machinify::Generate::current() const {
  const auto n = machine_->size_items()-1;
  auto* sb = static_cast<SeqBlock*>(machine_->back_items()->get_stmt());
  return make_pair(n, sb);
}

void Machinify::Generate::append(const Statement* s) {
  auto* sb = static_cast<SeqBlock*>(machine_->back_items()->get_stmt());
  append(sb, s);
}

void Machinify::Generate::append(SeqBlock* sb, const Statement* s) {
  auto* c = s->clone();
  sb->push_back_stmts(c);
}

void Machinify::Generate::transition(size_t n) {
  auto* sb = static_cast<SeqBlock*>(machine_->back_items()->get_stmt());
  transition(sb, n);
}

void Machinify::Generate::transition(SeqBlock* sb, size_t n) {
  sb->push_back_stmts(new NonblockingAssign(
    new Identifier(new Id("__state"), new Number(Bits(32, 0))),
    new Number(Bits(32, n))
  ));
}

void Machinify::Generate::next_state() {
  if (!machine_->empty_items()) {
    if (task_states_.empty() || (task_states_.back() != current().first)) {
      append(new NonblockingAssign(new Identifier(new Id("__task_id"), new Number(Bits(32, idx_))), new Number(Bits(32, 0))));
    }
  }
  auto* ci = new CaseItem(new SeqBlock());
  ci->push_back_exprs(new Number(Bits(32, machine_->size_items())));
  machine_->push_back_items(ci);
}

Identifier* Machinify::Generate::to_guard(const Event* e) const {
  assert(e->get_expr()->is(Node::Tag::identifier));
  const auto* i = static_cast<const Identifier*>(e->get_expr());
  switch (e->get_type()) {
    case Event::Type::NEGEDGE:
      return new Identifier(i->front_ids()->get_readable_sid()+"_negedge");
    case Event::Type::POSEDGE:
      return new Identifier(i->front_ids()->get_readable_sid()+"_posedge");
    default:
      assert(false);
      return nullptr;
  }
}

Machinify::~Machinify() {
  for (auto gen : generators_) {
    delete gen.text_;
  }
}

void Machinify::run(ModuleDeclaration* md) {
  for (auto i = md->begin_items(); i != md->end_items(); ) {
    // Ignore everything other than always constructs
    if (!(*i)->is(Node::Tag::always_construct)) {
      ++i;
      continue;
    }

    // Ignore combinational always constructs
    auto* ac = static_cast<AlwaysConstruct*>(*i);
    assert(ac->get_stmt()->is(Node::Tag::timing_control_statement));
    auto* tcs = static_cast<const TimingControlStatement*>(ac->get_stmt());
    assert(tcs->get_ctrl()->is(Node::Tag::event_control));
    auto* ec = static_cast<const EventControl*>(tcs->get_ctrl());
    if (ec->front_events()->get_type() == Event::Type::EDGE) {
      ++i;
      continue;
    }
      
    // Generate a state machine for this block and remove it from the AST.
    Generate gen(generators_.size());
    gen.run(ec, tcs->get_stmt());
    generators_.push_back(gen);
    i = md->purge_items(i);
  }
}

Machinify::const_iterator Machinify::begin() const {
  return generators_.begin();
}

Machinify::const_iterator Machinify::end() const {
  return generators_.end();
}

Machinify::TaskCheck::TaskCheck() : Visitor() { } 

bool Machinify::TaskCheck::run(const Node* n) {
  res_ = false; 
  n->accept(this);
  return res_;
}

void Machinify::TaskCheck::visit(const NonblockingAssign* na) {
  const auto* i = na->get_lhs();
  if (i->eq("__task_id")) {
    res_ = true;
  }
}

} // namespace cascade
