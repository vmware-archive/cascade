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

#ifndef CASCADE_SRC_TARGET_CORE_AVMM_MACHINIFY_H
#define CASCADE_SRC_TARGET_CORE_AVMM_MACHINIFY_H

#include <limits>
#include <stddef.h>
#include <utility>
#include <vector>
#include "verilog/ast/ast.h"
#include "verilog/ast/visitors/visitor.h"

namespace cascade {

// Pass 2: 
//        
// Removes edge-triggered always blocks from the AST and transforms them into
// continuation-passing state machines which can be combined later on into a
// single monolithic always @(posedge __clk) block. This pass uses system tasks
// as landmarks, but recall that they've been replaced by non-blocking assigns
// to __next_task_id in pass 1.

template <typename T>
class Machinify {
  public:
    // State Machine Construction Helpers:
    class Generate : public Visitor {
      public:
        Generate(size_t idx);
        ~Generate() override = default;

        const SeqBlock* text() const;
        size_t name() const;
        size_t final_state() const;

      private:
        friend class Machinify;

        SeqBlock* machine_;
        size_t idx_;
        std::pair<size_t, SeqBlock*> current_;

        void run(const EventControl* ec, const Statement* s);
        void visit(const BlockingAssign* ba) override;
        void visit(const NonblockingAssign* na) override;
        void visit(const SeqBlock* sb) override;
        void visit(const CaseStatement* cs) override;
        void visit(const ConditionalStatement* cs) override;

        // Copies a statement into the current state
        void append(const Statement* s);
        // Copies a statement into a sequential block
        void append(SeqBlock* sb, const Statement* s);
        // Appends a transition to state n to the current state
        void transition(size_t n);
        // Appends a transition to state n to a sequential block 
        void transition(SeqBlock* sb, size_t n);
        // Appends a new state to the state machine. If the machine is
        // non-empty and the current state is not a task state, appends a
        // non-blocking assign task_id <= 0;
        void next_state();
        // Transforms an event into a trigger signal
        Identifier* to_guard(const Event* e) const;
    };

    typedef typename std::vector<Generate>::const_iterator const_iterator;

    ~Machinify();

    void run(ModuleDeclaration* md);
    const_iterator begin() const;
    const_iterator end() const;

  private:
    // Checks whether an always construct contains any task statements which
    // require us to trap into the runtime.
    class TaskCheck : public Visitor {
      public:
        TaskCheck();
        ~TaskCheck() override = default;
        bool run(const Node* n);
      private:
        bool res_;
        void visit(const BlockingAssign* ba) override;
    };

    std::vector<Generate> generators_;
};

template <typename T>
inline Machinify<T>::Generate::Generate(size_t idx) : Visitor() { 
  idx_ = idx;
}

template <typename T>
inline const SeqBlock* Machinify<T>::Generate::text() const {
  return machine_;
}

template <typename T>
inline size_t Machinify<T>::Generate::name() const {
  return idx_;
}

template <typename T>
inline size_t Machinify<T>::Generate::final_state() const {
  return current_.first;
}

template <typename T>
inline void Machinify<T>::Generate::run(const EventControl* ec, const Statement* s) {
  // Create a state machine with a single state
  machine_ = new SeqBlock();
  next_state();

  // Populate the state machine
  s->accept(this);

  // Append a done state. If the last state in the machine is empty, we can use
  // that one. Otherwise, we'll create one last transition to an empty state.
  auto c = current_;
  if (!c.second->empty_stmts()) {
    transition(c.second, c.first+1);
    next_state();
    c = current_;
  }

  // Tie everything together into a conditional statement
  auto i = ec->begin_events();
  Expression* guard = to_guard(*i++);
  for (auto ie = ec->end_events(); i != ie; ++i) {
    guard = new BinaryExpression(to_guard(*i), BinaryExpression::Op::PPIPE, guard);
  }
  machine_->push_front_stmts(new ConditionalStatement(
    new Identifier("__continue"),
    new BlockingAssign(new Identifier(new Id("__task_id"), new Number(Bits(std::numeric_limits<T>::digits, idx_))), new Number(Bits(std::numeric_limits<T>::digits, 0))),
    new SeqBlock()));
  machine_->push_back_stmts(new BlockingAssign(
    new Identifier(new Id("__state"), new Number(Bits(std::numeric_limits<T>::digits, idx_))),
    new ConditionalExpression(
      new Identifier("__reset"),
      new Number(Bits(std::numeric_limits<T>::digits, final_state())),
      new ConditionalExpression(guard, new Number(Bits(false)), new Identifier(new Id("__state"), new Number(Bits(std::numeric_limits<T>::digits, idx_))))
    )
  ));
  machine_->push_back_stmts(new BlockingAssign(
    new Identifier(new Id("__task_id"), new Number(Bits(std::numeric_limits<T>::digits, idx_))),
    new ConditionalExpression(
      new Identifier("__reset"),
      new Number(Bits(std::numeric_limits<T>::digits, -1)),
      new Identifier(new Id("__task_id"), new Number(Bits(std::numeric_limits<T>::digits, idx_)))
    )
  ));
}

template <typename T>
inline void Machinify<T>::Generate::visit(const BlockingAssign* ba) {
  // Check whether this is a task
  const auto is_task = ba->get_lhs()->eq("__task_id");

  // If it's not, we can append it and move on. Otherwise, we need to record
  // the state that it appears in and mangle it a bit.
  if (!is_task) {
    append(ba);
  } else {
    auto* c = ba->clone();
    c->get_lhs()->push_front_dim(new Number(Bits(std::numeric_limits<T>::digits, idx_)));
    append(c);
    delete c;
  }

  // NOTE: We have the invariant that our code doesn't have any nested seq
  // blocks (which we didn't introduce ourselves, and by construction won't
  // have any tasks inside them).  
  
  // If this is the last statement in a seq block, it's already sitting at a
  // state boundary, and there's no need to introduce another break.
  const auto* p = ba->get_parent();
  if (p->is(Node::Tag::seq_block)) {
    const auto* sb = static_cast<const SeqBlock*>(p);
    if (sb->back_stmts() == ba) {
      return;
    }
  }
  // Otherwise, if this is a task, we'll need to break for a state transition.
  if (is_task) {
    transition(current_.first+1);
    next_state();
  }
}

template <typename T>
inline void Machinify<T>::Generate::visit(const NonblockingAssign* na) {
  append(na);
}

template <typename T>
inline void Machinify<T>::Generate::visit(const SeqBlock* sb) {
  sb->accept_stmts(this);
}

template <typename T>
inline void Machinify<T>::Generate::visit(const CaseStatement* cs) {
  // TODO(eschkufz) There are similar optimizations to the ones in
  // ConditionalStatement that can still be made here.

  if (!TaskCheck().run(cs)) {
    append(cs);
    return;
  } 

  const auto begin = current_;

  std::vector<std::pair<size_t, SeqBlock*>> begins;
  std::vector<std::pair<size_t, SeqBlock*>> ends;
  for (auto i = cs->begin_items(), ie = cs->end_items(); i != ie; ++i) {
    next_state();
    begins.push_back(current_);
    (*i)->accept_stmt(this);
    ends.push_back(current_);
  }

  auto* branch = new CaseStatement(cs->get_type(), cs->get_cond()->clone());
  size_t idx = 0;
  for (auto i = cs->begin_items(), ie = cs->end_items(); i != ie; ++i) {
    branch->push_back_items(new CaseItem(
      new BlockingAssign(
        new Identifier(new Id("__state"), new Number(Bits(std::numeric_limits<T>::digits, idx_))),
        new Number(Bits(std::numeric_limits<T>::digits, begins[idx++].first))
      )
    ));
    for (auto j = (*i)->begin_exprs(), je = (*i)->end_exprs(); j != je; ++j) {
      branch->back_items()->push_back_exprs((*j)->clone());
    }
  }
  append(begin.second, branch);
  
  next_state();
  for (auto& e : ends) {
    transition(e.second, current_.first);
  }
}

template <typename T>
inline void Machinify<T>::Generate::visit(const ConditionalStatement* cs) {
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
  const auto begin = current_;

  // We definitely need a new state for the true branch
  next_state();
  const auto then_begin = current_;
  cs->get_then()->accept(this);
  const auto then_end = current_;

  // We only need a new state for the else branch if it's non-empty.
  if (!empty_else) {
    next_state();
  }
  const auto else_begin = current_;
  cs->get_else()->accept(this);
  const auto else_end = current_;

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
    new BlockingAssign(
      new Identifier(new Id("__state"), new Number(Bits(std::numeric_limits<T>::digits, idx_))),
      new Number(Bits(std::numeric_limits<T>::digits, then_begin.first))
    ),
    new BlockingAssign(
      new Identifier(new Id("__state"), new Number(Bits(std::numeric_limits<T>::digits, idx_))),
      new Number(Bits(std::numeric_limits<T>::digits, !empty_else ? else_begin.first : (then_end.first + 1)))
    )
  );
  append(begin.second, branch);

  // If we emitted a phi node, the then branch goes there (to the current state).
  // And if the else branch was non-empty, it goes there as well.
  if (phi_node) {
    transition(then_end.second, current_.first);
    if (!empty_else) {
      transition(else_end.second, current_.first);
    }
  }
}

template <typename T>
inline void Machinify<T>::Generate::append(const Statement* s) {
  append(current_.second, s);
}

template <typename T>
inline void Machinify<T>::Generate::append(SeqBlock* sb, const Statement* s) {
  auto* c = s->clone();
  sb->push_back_stmts(c);
}

template <typename T>
inline void Machinify<T>::Generate::transition(size_t n) {
  transition(current_.second, n);
}

template <typename T>
inline void Machinify<T>::Generate::transition(SeqBlock* sb, size_t n) {
  sb->push_back_stmts(new BlockingAssign(
    new Identifier(new Id("__state"), new Number(Bits(std::numeric_limits<T>::digits, 0))),
    new Number(Bits(std::numeric_limits<T>::digits, n))
  ));
}

template <typename T>
inline void Machinify<T>::Generate::next_state() {
  auto state = machine_->empty_stmts() ? 0 : (current_.first + 1);
  auto* cs = new ConditionalStatement(
    new BinaryExpression(
      new BinaryExpression(new Identifier(new Id("__state"), new Number(Bits(std::numeric_limits<T>::digits, idx_))), BinaryExpression::Op::EEQ, new Number(Bits(std::numeric_limits<T>::digits, state))),
      BinaryExpression::Op::AAMP,
      new BinaryExpression(new Identifier(new Id("__task_id"), new Number(Bits(std::numeric_limits<T>::digits, idx_))), BinaryExpression::Op::EEQ, new Number(Bits(std::numeric_limits<T>::digits, 0)))
    ),
    new SeqBlock(),
    new SeqBlock()
  );
  machine_->push_back_stmts(cs);

  current_ = std::make_pair(machine_->size_stmts()-1, static_cast<SeqBlock*>(cs->get_then()));
}

template <typename T>
inline Identifier* Machinify<T>::Generate::to_guard(const Event* e) const {
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

template <typename T>
inline Machinify<T>::~Machinify<T>() {
  for (auto gen : generators_) {
    delete gen.machine_;
  }
}

template <typename T>
inline void Machinify<T>::run(ModuleDeclaration* md) {
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

template <typename T>
inline typename Machinify<T>::const_iterator Machinify<T>::begin() const {
  return generators_.begin();
}

template <typename T>
inline typename Machinify<T>::const_iterator Machinify<T>::end() const {
  return generators_.end();
}

template <typename T>
inline Machinify<T>::TaskCheck::TaskCheck() : Visitor() { } 

template <typename T>
inline bool Machinify<T>::TaskCheck::run(const Node* n) {
  res_ = false; 
  n->accept(this);
  return res_;
}

template <typename T>
inline void Machinify<T>::TaskCheck::visit(const BlockingAssign* ba) {
  const auto* i = ba->get_lhs();
  if (i->eq("__task_id")) {
    res_ = true;
  }
}

} // namespace cascade

#endif

