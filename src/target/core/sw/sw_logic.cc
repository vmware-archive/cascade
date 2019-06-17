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

#include "target/core/sw/sw_logic.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include "target/core/common/interfacestream.h"
#include "target/core/common/printf.h"
#include "target/core/common/scanf.h"
#include "target/core/sw/monitor.h"
#include "target/input.h"
#include "target/state.h"
#include "verilog/analyze/module_info.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"
#include "verilog/print/text/text_printer.h"

using namespace std;

namespace cascade {

SwLogic::SwLogic(Interface* interface, ModuleDeclaration* md) : Logic(interface), Visitor() { 
  // Record pointer to source code and provision update pool
  src_ = md;
  update_pool_.resize(1);

  // Initialize monitors and system tasks
  for (auto i = src_->begin_items(), ie = src_->end_items(); i != ie; ++i) {
    Monitor().init(*i);
  }
  eval_.set_feof_handler([this](Evaluate* eval, const FeofExpression* fe) {
    const auto fd = eval_.get_value(fe->get_fd()).to_int();
    return get_stream(fd)->eof();
  });
  EofIndex ei(this);
  src_->accept(&ei);

  // Set silent mode, schedule always constructs and continuous assigns, and then
  // place the silent flag in its default, disabled state
  silent_ = true;
  for (auto i = src_->begin_items(), ie = src_->end_items(); i != ie; ++i) {
    if ((*i)->is(Node::Tag::always_construct) || (*i)->is(Node::Tag::continuous_assign)) {
      schedule_now(*i);
    }
  }
  silent_ = false;
}

SwLogic::~SwLogic() {
  delete src_;
  for (auto& s : streams_) {
    delete s.second;
  }
}

SwLogic& SwLogic::set_input(const Identifier* id, VId vid) {
  if (vid >= inputs_.size()) {
    inputs_.resize(vid+1, nullptr);
  }
  inputs_[vid] = id;
  return *this;
}

SwLogic& SwLogic::set_state(const Identifier* id, VId vid) {
  state_.insert(make_pair(vid, id));
  return *this;
}

SwLogic& SwLogic::set_output(const Identifier* id, VId vid) {
  outputs_.push_back(make_pair(id, vid));
  return *this;
}

State* SwLogic::get_state() {
  auto* s = new State();
  for (const auto& sv : state_) {
    s->insert(sv.first, eval_.get_array_value(sv.second));
  }
  return s;
}

void SwLogic::set_state(const State* s) {
  for (const auto& sv : state_) {
    const auto itr = s->find(sv.first);
    if (itr != s->end()) {
      eval_.assign_array_value(sv.second, itr->second);
      notify(sv.second);
    }
  }
  silent_evaluate();
}

Input* SwLogic::get_input() {
  auto* i = new Input();
  for (size_t v = 0, ve = inputs_.size(); v < ve; ++v) {
    const auto* id = inputs_[v];
    if (id == nullptr) {
      continue;
    }
    i->insert(v, eval_.get_value(id));
  }
  return i;
}

void SwLogic::set_input(const Input* i) {
  for (size_t v = 0, ve = inputs_.size(); v < ve; ++v) {
    const auto* id = inputs_[v];
    if (id == nullptr) {
      continue;
    }
    const auto itr = i->find(v);
    if (itr != i->end()) {
      eval_.assign_value(id, itr->second);
    }
    notify(id);
  }
  silent_evaluate();
}

void SwLogic::finalize() {
  // Handle calls to fopen.
  for (auto i = src_->begin_items(), ie = src_->end_items(); i != ie; ++i) {
    if ((*i)->is(Node::Tag::reg_declaration)) {
      const auto* rd = static_cast<const RegDeclaration*>(*i);
      if (rd->is_non_null_val() && rd->get_val()->is(Node::Tag::fopen_expression)) {
        const auto* fe = static_cast<const FopenExpression*>(rd->get_val());
        if (eval_.get_value(rd->get_id()).to_int() == 0) {
          const auto fd = interface()->fopen(eval_.get_value(fe->get_path()).to_str());
          eval_.assign_value(rd->get_id(), Bits(32, fd));
          notify(rd->get_id());
        }
      }
    }
  }
  // Schedule initial constructs
  for (auto i = src_->begin_items(), ie = src_->end_items(); i != ie; ++i) {
    if ((*i)->is(Node::Tag::initial_construct)) {
      schedule_now(*i);
    }
  }
}

void SwLogic::read(VId vid, const Bits* b) {
  const auto* id = inputs_[vid];
  eval_.assign_value(id, *b);
  notify(id);
}

void SwLogic::evaluate() {
  // This is a while loop. Active events can generate new active events.
  there_were_tasks_ = false;
  while (!active_.empty()) {
    auto* e = active_.back();
    active_.pop_back();
    const_cast<Node*>(e)->set_flag<1>(false);
    schedule_now(e);
  }
  for (auto& o : outputs_) {
    interface()->write(o.second, &eval_.get_value(o.first));
  }
}

bool SwLogic::there_are_updates() const {
  return !updates_.empty();
}

void SwLogic::update() {
  // This is a for loop. Updates happen simultaneously
  for (size_t i = 0, ie = updates_.size(); i < ie; ++i) {
    const auto& val = update_pool_[i];
    eval_.assign_value(get<0>(updates_[i]), get<1>(updates_[i]), get<2>(updates_[i]), get<3>(updates_[i]), val);
    notify(get<0>(updates_[i]));
  }
  updates_.clear();

  // This is while loop. Active events can generate new active events.
  there_were_tasks_ = false;
  while (!active_.empty()) {
    auto* e = active_.back();
    active_.pop_back();
    const_cast<Node*>(e)->set_flag<1>(false);
    schedule_now(e);
  }

  for (auto& o : outputs_) {
    interface()->write(o.second, &eval_.get_value(o.first));
  }
}

bool SwLogic::there_were_tasks() const {
  return there_were_tasks_;
}

SwLogic::EofIndex::EofIndex(SwLogic* sw) : Visitor() {
  sw_ = sw;
}

void SwLogic::EofIndex::visit(const FeofExpression* fe) {
  sw_->eofs_.push_back(fe);
}

void SwLogic::schedule_now(const Node* n) {
  n->accept(this);
}

void SwLogic::schedule_active(const Node* n) {
  if (!n->get_flag<1>()) {
    active_.push_back(n);
    const_cast<Node*>(n)->set_flag<1>(true);
  }
}

void SwLogic::notify(const Node* n) {
  switch (n->get_tag()) {
    case Node::Tag::identifier:
      for (auto* m : static_cast<const Identifier*>(n)->monitor_) {
        schedule_active(m);
      }
      return;
    case Node::Tag::feof_expression:
      for (auto* m : static_cast<const FeofExpression*>(n)->monitor_) {
        schedule_active(m);
      }
      return;
    default:
      break;
  }
  const auto* p = n->get_parent();
  if (p->is(Node::Tag::case_item)) {
    schedule_active(p->get_parent());
  } else if (!p->is(Node::Tag::initial_construct)) {
    schedule_active(p); 
  }
}

void SwLogic::silent_evaluate() {
  // Turn on silent mode and drain the active queue
  silent_ = true;
  while (!active_.empty()) {
    auto* e = active_.back();
    active_.pop_back();
    const_cast<Node*>(e)->set_flag<1>(false);
    schedule_now(e);
  }
  silent_ = false;
}

uint16_t& SwLogic::get_state(const Statement* s) {
  return const_cast<Statement*>(s)->ctrl_;
}

interfacestream* SwLogic::get_stream(FId fd) {
  const auto itr = streams_.find(fd);
  if (itr != streams_.end()) {
    return itr->second;
  }
  auto* is = new interfacestream(interface(), fd);
  streams_[fd] = is;
  return is;
}

void SwLogic::visit(const Event* e) {
  // TODO(eschkufz) Support for complex expressions 
  assert(e->get_expr()->is(Node::Tag::identifier));
  const auto* id = static_cast<const Identifier*>(e->get_expr());
  const auto* r = Resolve().get_resolution(id);

  if (e->get_type() != Event::Type::NEGEDGE && eval_.get_value(r).to_bool()) {
    notify(e);
  } else if (e->get_type() != Event::Type::POSEDGE && !eval_.get_value(r).to_bool()) {
    notify(e);
  }
}

void SwLogic::visit(const AlwaysConstruct* ac) {
  schedule_now(ac->get_stmt());
}

void SwLogic::visit(const InitialConstruct* ic) {
  schedule_active(ic->get_stmt());
}

void SwLogic::visit(const ContinuousAssign* ca) {
  // TODO(eschkufz) Support for timing control
  assert(ca->is_null_ctrl());
  schedule_now(ca->get_assign());
}

void SwLogic::visit(const BlockingAssign* ba) {
  // TODO(eschkufz) Support for timing control
  assert(ba->is_null_ctrl());

  schedule_now(ba->get_assign());
  notify(ba);
}

void SwLogic::visit(const NonblockingAssign* na) {
  // TODO(eschkufz) Support for timing control
  assert(na->is_null_ctrl());
  
  if (!silent_) {
    const auto* r = Resolve().get_resolution(na->get_assign()->get_lhs());
    assert(r != nullptr);
    const auto target = eval_.dereference(r, na->get_assign()->get_lhs());
    const auto& res = eval_.get_value(na->get_assign()->get_rhs());

    const auto idx = updates_.size();
    if (idx >= update_pool_.size()) {
      update_pool_.resize(2*update_pool_.size());
    } 

    updates_.push_back(make_tuple(r, get<0>(target), get<1>(target), get<2>(target)));
    update_pool_[idx] = res;
  }
  notify(na);
}

void SwLogic::visit(const SeqBlock* sb) { 
  auto& state = get_state(sb);
  if (state < sb->size_stmts()) {
    auto* item = sb->get_stmts(state++);
    schedule_now(item);
  } else {
    state = 0;
    notify(sb);
  }
}

void SwLogic::visit(const CaseStatement* cs) {
  auto& state = get_state(cs);
  if (state == 0) {
    state = 1;
    const auto s = eval_.get_value(cs->get_cond()).to_int();
    for (auto i = cs->begin_items(), ie = cs->end_items(); i != ie; ++i) { 
      for (auto j = (*i)->begin_exprs(), je = (*i)->end_exprs(); j != je; ++j) { 
        const auto c = eval_.get_value(*j).to_int();
        if (s == c) {
          schedule_now((*i)->get_stmt());
          return;
        }
      } 
      if ((*i)->empty_exprs()) {
        schedule_now((*i)->get_stmt());
        return;
      }
    }
    // Control should never reach here
    assert(false);
  } else {
    state = 0;
    notify(cs);
  }
}

void SwLogic::visit(const ConditionalStatement* cs) {
  auto& state = get_state(cs);
  if (state == 0) {
    state = 1;
    if (eval_.get_value(cs->get_if()).to_bool()) {
      schedule_now(cs->get_then());
    } else {
      schedule_now(cs->get_else());
    }
  } else {
    state = 0;
    notify(cs);
  }
}

void SwLogic::visit(const TimingControlStatement* tcs) {
  auto& state = get_state(tcs);
  switch (state) {
    case 0:
      state = 1;
      // Wait on control 
      break;
    case 1:
      state = 2;
      schedule_now(tcs->get_stmt());
      break;
    default:
      state = 0;
      notify(tcs);
      break;
  }
}

void SwLogic::visit(const FflushStatement* fs) {
  if (!silent_) {
    const auto fd = eval_.get_value(fs->get_fd()).to_int();
    auto* is = get_stream(fd);
    is->flush();
  }
  notify(fs);
}

void SwLogic::visit(const FinishStatement* fs) {
  if (!silent_) {
    interface()->finish(eval_.get_value(fs->get_arg()).to_int());
    there_were_tasks_ = true;
  }
  notify(fs);
}

void SwLogic::visit(const FseekStatement* fs) {
  if (!silent_) {
    const auto fd = eval_.get_value(fs->get_fd()).to_int();
    auto* is = get_stream(fd);

    const auto offset = eval_.get_value(fs->get_offset()).to_int();
    const auto op = eval_.get_value(fs->get_op()).to_int();
    const auto way = (op == 0) ? ios_base::beg : (op == 1) ? ios_base::cur : ios_base::end;

    is->clear();
    is->seekg(offset, way); 
    is->seekp(offset, way);

    for (auto* fe : eofs_) {
      eval_.flag_changed(fe);
      notify(fe);
    }
  }
  notify(fs);
}

void SwLogic::visit(const GetStatement* gs) {
  if (!silent_) {
    const auto fd = eval_.get_value(gs->get_fd()).to_int();
    auto* is = get_stream(fd);
    Scanf().read(*is, &eval_, gs);

    if (gs->is_non_null_var()) {
      const auto* r = Resolve().get_resolution(gs->get_var());
      assert(r != nullptr);
      notify(r);
    }
    for (auto* fe : eofs_) {
      eval_.flag_changed(fe);
      notify(fe);
    }
  }
  notify(gs);
}

void SwLogic::visit(const PutStatement* ps) {
  if (!silent_) {
    const auto fd = eval_.get_value(ps->get_fd()).to_int();
    auto* is = get_stream(fd);
    Printf().write(*is, &eval_, ps);

    for (auto* fe : eofs_) {
      eval_.flag_changed(fe);
      notify(fe);
    }
  }
  notify(ps);
}

void SwLogic::visit(const RestartStatement* rs) {
  if (!silent_) {
    interface()->restart(rs->get_arg()->get_readable_val());
    there_were_tasks_ = true;
  }
  notify(rs);
}

void SwLogic::visit(const RetargetStatement* rs) {
  if (!silent_) {
    interface()->retarget(rs->get_arg()->get_readable_val());
    there_were_tasks_ = true;
  }
  notify(rs);
}

void SwLogic::visit(const SaveStatement* ss) {
  if (!silent_) {
    interface()->save(ss->get_arg()->get_readable_val());
    there_were_tasks_ = true;
  }
  notify(ss);
}

void SwLogic::visit(const DelayControl* dc) {
  // TODO(eschkufz) Support for delay control
  assert(false);
  (void) dc;
}

void SwLogic::visit(const EventControl* ec) {
  notify(ec);
}

void SwLogic::visit(const VariableAssign* va) {
  const auto& res = eval_.get_value(va->get_rhs());
  eval_.assign_value(va->get_lhs(), res);
  notify(Resolve().get_resolution(va->get_lhs()));
}

void SwLogic::log(const string& op, const Node* n) {
  TextPrinter(cout) << "[" << src_->get_id() << "] " << op << " " << n << "\n";
}

} // namespace cascade
