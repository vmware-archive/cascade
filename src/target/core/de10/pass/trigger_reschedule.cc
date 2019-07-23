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

#include "target/core/de10/pass/trigger_reschedule.h"

#include "verilog/ast/ast.h"

namespace cascade {

TriggerReschedule::TriggerReschedule() : Editor() { }

void TriggerReschedule::edit(AlwaysConstruct* ac) {
  assert(ac->get_stmt()->is(Node::Tag::timing_control_statement));
  auto* tcs = static_cast<TimingControlStatement*>(ac->get_stmt());
  assert(tcs->get_ctrl()->is(Node::Tag::event_control));
  auto* ec = static_cast<EventControl*>(tcs->get_ctrl());

  // Nothing to do if this is a combinational always block
  if (ec->front_events()->get_type() == Event::Type::EDGE) {
    return;
  }
  
  // Check whether this block was machinified --- it will have a non-null id.
  const auto is_sb = tcs->get_stmt()->is(Node::Tag::seq_block);
  const auto is_mified = is_sb && static_cast<SeqBlock*>(tcs->get_stmt())->is_non_null_id();

  // Push triggers down into a guard
  auto i = ec->begin_events();
  Expression* guard = to_guard(*i++);
  for (auto ie = ec->end_events(); i != ie; ++i) {
    guard = new BinaryExpression(to_guard(*i), BinaryExpression::Op::PPIPE, guard);
  }

  // Create a new block. If this code was machinified, we'll need to copy it's
  // id and declarations up to here
  auto* sb = new SeqBlock();
  if (is_mified) {
    auto* rhs = static_cast<SeqBlock*>(tcs->get_stmt());
    sb->set_id(rhs->remove_id());
    while (!rhs->empty_decls()) {
      sb->push_back_decls(rhs->remove_front_decls());
    }
  }
  // Create the control logic for the new block
  if (!is_mified) {
    sb->push_back_stmts(new ConditionalStatement(guard, tcs->get_stmt()->clone(), new SeqBlock()));
  } else {
    sb->push_back_stmts(new ConditionalStatement(
      new Identifier("__continue"), 
      static_cast<SeqBlock*>(tcs->get_stmt())->front_stmts()->clone(),
      new SeqBlock(
        new NonblockingAssign(
          new Identifier("__state"),
          new ConditionalExpression(
            new Identifier("__reset"),
            new Identifier("__final"),
            new ConditionalExpression(
              guard,
              new Number(Bits(false)),
              new Identifier("__state")
            )
          )
        ))
    ));
  }

  // Push the new block down into a new always @(posedge __clk)        
  tcs->replace_ctrl(new EventControl(new Event(Event::Type::POSEDGE, new Identifier("__clk"))));
  tcs->replace_stmt(sb);
}

Identifier* TriggerReschedule::to_guard(const Event* e) const {
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

} // namespace cascade
