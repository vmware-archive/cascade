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

#include "src/target/core/de10/machinify.h"

#include <sstream>
#include <vector>
#include "src/base/bits/bits.h"
#include "src/verilog/ast/ast.h"

using namespace std;

namespace cascade {

Machinify::Machinify() : Builder() { }

ModuleDeclaration* Machinify::run(const ModuleDeclaration* md)  {
  idx_ = 0;
  return md->accept(this);
}

ModuleDeclaration* Machinify::build(const ModuleDeclaration* md) {
  auto* res = Builder::build(md);
  res->push_front_items(new RegDeclaration(
    new Attributes(),
    new Identifier("__resume"),
    false,
    nullptr,
    new Number(Bits(false))
  ));
  for (size_t i = 0; i < idx_; ++i) {
    stringstream ss;
    ss << "__state_" << i;
    res->push_front_items(new RegDeclaration(
      new Attributes(),
      new Identifier(ss.str()),
      false,
      new RangeExpression(32, 0),
      new Number(Bits(32, 0))
    ));
  }
  return res;
}

ModuleItem* Machinify::build(const AlwaysConstruct* ac) {
  assert(ac->get_stmt()->is(Node::Tag::timing_control_statement));
  const auto* tcs = static_cast<const TimingControlStatement*>(ac->get_stmt());
  assert(tcs->get_ctrl()->is(Ndoe::Tag::event_control));
  const auto* ec = static_cast<const EventControl*>(tcs->get_ctrl());

  auto* ctrl = ec->clone();
  ctrl->push_back_events(new Event(Event::Type::POSEDGE, new Identifier("__resume")));
  auto* machine = Generate(idx_++).run(tcs->get_stmt());
  return new AlwaysConstruct(new TimingControlStatement(ctrl, machine));
}

Machinify::Generate::Generate(size_t idx) : Visitor() { 
  idx_ = idx;
}

CaseStatement* Machinify::Generate::run(const Statement* s) {
  machine_ = new CaseStatement(CaseStatement::Type::CASE, state_var());
  next_state();
  s->accept(this);
  return machine_;
}

void Machinify::Generate::visit(const BlockingAssign* ba) {
  append(ba);
}

void Machinify::Generate::visit(const NonblockingAssign* na) {
  append(na);
}

void Machinify::Generate::visit(const SeqBlock* sb) {
  sb->accept_stmts(this);
}

void Machinify::Generate::visit(const CaseStatement* cs) {
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
      new NonblockingAssign(new VariableAssign(
        new Identifier("state"),
        new Number(Bits(32, begins[idx++].first))
      ))
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
  const auto begin = current();

  next_state();
  const auto then_begin = current();
  cs->get_then()->accept(this);
  const auto then_end = current();
  
  next_state();
  const auto else_begin = current();
  cs->get_else()->accept(this);
  const auto else_end = current();
  
  auto* branch = new ConditionalStatement(
    cs->get_if()->clone(),
    new NonblockingAssign(new VariableAssign(
      new Identifier("state"),
      new Number(Bits(32, then_begin.first))
    )),
    new NonblockingAssign(new VariableAssign(
      new Identifier("state"),
      new Number(Bits(32, else_begin.first))
    ))
  );
  append(begin.second, branch);

  next_state();
  transition(then_end.second, current().first);
  transition(else_end.second, current().first);
}

void Machinify::Generate::visit(const DisplayStatement* ds) {
  append(ds);
}

void Machinify::Generate::visit(const ErrorStatement* es) {
  append(es);
}

void Machinify::Generate::visit(const FinishStatement* fs) {
  append(fs);
}

void Machinify::Generate::visit(const GetStatement* gs) {
  append(gs);
  transition(current().first+1);
  next_state();
}

void Machinify::Generate::visit(const InfoStatement* is) {
  append(is);
}

void Machinify::Generate::visit(const PutStatement* ps) {
  append(ps);
}

void Machinify::Generate::visit(const RestartStatement* rs) {
  append(rs);
}

void Machinify::Generate::visit(const RetargetStatement* rs) {
  append(rs);
}

void Machinify::Generate::visit(const SaveStatement* ss) {
  append(ss);
}

void Machinify::Generate::visit(const SeekStatement* ss) {
  append(ss);
  transition(current().first+1);
  next_state();
}

void Machinify::Generate::visit(const WarningStatement* ws) {
  append(ws);
}

void Machinify::Generate::visit(const WriteStatement* ws) {
  append(ws);
}

pair<size_t, SeqBlock*> Machinify::Generate::current() const {
  const auto n = machine_->size_items()-1;
  auto* sb = static_cast<SeqBlock*>(machine_->back_items()->get_stmt());
  return make_pair(n, sb);
}

Identifier* Machinify::Generate::state_var() const {
  stringstream ss;
  ss << "__state_" << idx_;
  return new Identifier(ss.str());
}

void Machinify::Generate::append(const Statement* s) {
  auto* sb = static_cast<SeqBlock*>(machine_->back_items()->get_stmt());
  append(sb, s);
}

void Machinify::Generate::append(SeqBlock* sb, const Statement* s) {
  sb->push_back_stmts(s->clone());
}

void Machinify::Generate::transition(size_t n) {
  auto* sb = static_cast<SeqBlock*>(machine_->back_items()->get_stmt());
  transition(sb, n);
}

void Machinify::Generate::transition(SeqBlock* sb, size_t n) {
  sb->push_back_stmts(new NonblockingAssign(new VariableAssign(
    state_var(),
    new Number(Bits(32, n))
    )));
}

void Machinify::Generate::next_state() {
  auto* ci = new CaseItem(new SeqBlock());
  ci->push_back_exprs(new Number(Bits(32, machine_->size_items())));
  machine_->push_back_items(ci);
}

} // namespace cascade
