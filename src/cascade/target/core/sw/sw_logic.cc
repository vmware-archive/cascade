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
#include "verilog/print/print.h"

using namespace std;

namespace cascade::sw {

SwLogic::SwLogic(Interface* interface, ModuleDeclaration* md) : Logic(interface), Visitor() { 
  // Record pointer to source code and provision update pool
  src_ = md;
  update_pool_.resize(1);

  // Initialize monitors and system tasks
  for (auto i = src_->begin_items(), ie = src_->end_items(); i != ie; ++i) {
    Monitor().init(*i);
  }
  eval_.set_feof_handler([this](Evaluate* eval, const FeofExpression* fe) {
    const auto fd = eval_.get_value(fe->get_fd()).to_uint();
    return get_stream(fd)->eof();
  });
  EofIndex ei(this);
  src_->accept(&ei);

  // Set silent mode, schedule always constructs and continuous assigns, and then
  // place the silent flag in its default, disabled state
  silent_ = true;
  for (auto i = src_->begin_items(), ie = src_->end_items(); i != ie; ++i) {
    if ((*i)->is(Node::Tag::continuous_assign)) {
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
      if (eval_.assign_value(id, itr->second)) {
        notify(id);
      }
    }
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
        if (eval_.get_value(rd->get_id()).to_uint() == 0) {
          const auto path = eval_.get_value(fe->get_path()).to_string();
          const auto type = eval_.get_value(fe->get_type()).to_string();
          uint8_t mode = 0;
          if (type == "r" || type == "rb") {
            mode = 0;
          } else if (type == "w" || type == "wb") {
            mode = 1;
          } else if (type == "a" || type == "ab") {
            mode = 2;
          } else if (type == "r+" || type == "r+b" || type == "rb+") {
            mode = 3;
          } else if (type == "w+" || type == "w+b" || type == "wb+") {
            mode = 4;
          } else if (type == "a+" || type == "a+b" || type == "ab+") {
            mode = 5;
          } 
          const auto fd = interface()->fopen(path, mode);
          if (eval_.assign_value(rd->get_id(), Bits(32, fd))) {
            notify(rd->get_id());
          }
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
  if (eval_.assign_value(id, *b)) {
    notify(id);
  }
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
    if (eval_.assign_value(get<0>(updates_[i]), get<1>(updates_[i]), get<2>(updates_[i]), get<3>(updates_[i]), val)) {
      notify(get<0>(updates_[i]));
    }
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
      break;
    case Node::Tag::feof_expression:
      for (auto* m : static_cast<const FeofExpression*>(n)->monitor_) {
        schedule_active(m);
      }
      break;
    case Node::Tag::event:
      assert(n->get_parent()->is(Node::Tag::event_control));
      assert(n->get_parent()->get_parent()->is(Node::Tag::timing_control_statement));
      schedule_active(static_cast<const TimingControlStatement*>(n->get_parent()->get_parent())->get_stmt());
      return;
    default:
      break;
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

interfacestream* SwLogic::get_stream(FId fd) {
  const auto itr = streams_.find(fd);
  if (itr != streams_.end()) {
    return itr->second;
  }
  auto* is = new interfacestream(interface(), fd);
  streams_[fd] = is;
  return is;
}

void SwLogic::update_eofs() {
  for (auto* fe : eofs_) {
    eval_.flag_changed(fe);
    notify(fe);
  }
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

void SwLogic::visit(const ContinuousAssign* ca) {
  const auto& val = eval_.get_value(ca->get_rhs());
  if (eval_.assign_value(ca->get_lhs(), val)) {
    notify(Resolve().get_resolution(ca->get_lhs()));
  }
}

void SwLogic::visit(const BlockingAssign* ba) {
  // TODO(eschkufz) Support for timing control
  assert(ba->is_null_ctrl());

  const auto& res = eval_.get_value(ba->get_rhs());
  if (eval_.assign_value(ba->get_lhs(), res)) {
    notify(Resolve().get_resolution(ba->get_lhs()));
  }
}

void SwLogic::visit(const NonblockingAssign* na) {
  // TODO(eschkufz) Support for timing control
  assert(na->is_null_ctrl());
  
  if (!silent_) {
    const auto* r = Resolve().get_resolution(na->get_lhs());
    assert(r != nullptr);
    const auto target = eval_.dereference(r, na->get_lhs());
    const auto& res = eval_.get_value(na->get_rhs());

    const auto idx = updates_.size();
    if (idx >= update_pool_.size()) {
      update_pool_.resize(2*update_pool_.size());
    } 

    updates_.push_back(make_tuple(r, get<0>(target), get<1>(target), get<2>(target)));
    update_pool_[idx].copy(res);
  }
}

void SwLogic::visit(const SeqBlock* sb) { 
  for (auto i = sb->begin_stmts(), ie = sb->end_stmts(); i != ie; ++i) {
    schedule_now(*i);
  }
}

void SwLogic::visit(const CaseStatement* cs) {
  const auto s = eval_.get_value(cs->get_cond()).to_uint();
  for (auto i = cs->begin_items(), ie = cs->end_items(); i != ie; ++i) { 
    for (auto j = (*i)->begin_exprs(), je = (*i)->end_exprs(); j != je; ++j) { 
      const auto c = eval_.get_value(*j).to_uint();
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
}

void SwLogic::visit(const ConditionalStatement* cs) {
  if (eval_.get_value(cs->get_if()).to_bool()) {
    schedule_now(cs->get_then());
  } else {
    schedule_now(cs->get_else());
  }
}

void SwLogic::visit(const FflushStatement* fs) {
  if (!silent_) {
    const auto fd = eval_.get_value(fs->get_fd()).to_uint();
    auto* is = get_stream(fd);
    is->clear();
    is->flush();
    update_eofs();
  }
}

void SwLogic::visit(const FinishStatement* fs) {
  if (!silent_) {
    interface()->finish(eval_.get_value(fs->get_arg()).to_uint());
    there_were_tasks_ = true;
  }
}

void SwLogic::visit(const FseekStatement* fs) {
  if (!silent_) {
    const auto fd = eval_.get_value(fs->get_fd()).to_uint();
    auto* is = get_stream(fd);

    const auto offset = eval_.get_value(fs->get_offset()).to_uint();
    const auto op = eval_.get_value(fs->get_op()).to_uint();
    const auto way = (op == 0) ? ios_base::beg : (op == 1) ? ios_base::cur : ios_base::end;

    is->clear();
    is->seekg(offset, way); 
    is->seekp(offset, way);
    update_eofs();
  }
}

void SwLogic::visit(const DebugStatement* ds) {
  if (!silent_) {
    stringstream ss;
    ss << ds->get_arg();
    interface()->debug(Evaluate().get_value(ds->get_action()).to_uint(), ss.str());
    there_were_tasks_ = true;
  }
}

void SwLogic::visit(const GetStatement* gs) {
  if (!silent_) {
    const auto fd = eval_.get_value(gs->get_fd()).to_uint();
    auto* is = get_stream(fd);
    Scanf().read(*is, &eval_, gs);

    if (gs->is_non_null_var()) {
      const auto* r = Resolve().get_resolution(gs->get_var());
      assert(r != nullptr);
      notify(r);
    }
    update_eofs();
  }
}

void SwLogic::visit(const PutStatement* ps) {
  if (!silent_) {
    const auto fd = eval_.get_value(ps->get_fd()).to_uint();
    auto* is = get_stream(fd);
    Printf().write(*is, &eval_, ps);
    update_eofs();
  }
}

void SwLogic::visit(const RestartStatement* rs) {
  if (!silent_) {
    interface()->restart(rs->get_arg()->get_readable_val());
    there_were_tasks_ = true;
  }
}

void SwLogic::visit(const RetargetStatement* rs) {
  if (!silent_) {
    interface()->retarget(rs->get_arg()->get_readable_val());
    there_were_tasks_ = true;
  }
}

void SwLogic::visit(const SaveStatement* ss) {
  if (!silent_) {
    interface()->save(ss->get_arg()->get_readable_val());
    there_were_tasks_ = true;
  }
}

void SwLogic::log(const string& op, const Node* n) {
  cout << "[" << src_->get_id() << "] " << op << " " << n << endl;
}

} // namespace cascade::sw
