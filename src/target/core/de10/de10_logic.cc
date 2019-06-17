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

#include "target/core/de10/de10_logic.h"

#include <cassert>
#include "target/core/common/interfacestream.h"
#include "target/core/common/printf.h"
#include "target/core/common/scanf.h"
#include "target/core/de10/de10_compiler.h"
#include "target/input.h"
#include "target/state.h"
#include "verilog/ast/ast.h"
#include "verilog/analyze/evaluate.h"
#include "verilog/analyze/module_info.h"

using namespace std;

namespace cascade {

De10Logic::De10Logic(Interface* interface, QuartusServer::Id id, ModuleDeclaration* src, volatile uint8_t* addr) : Logic(interface), table_(addr) { 
  id_ = id;
  src_ = src;
}

De10Logic::~De10Logic() {
  delete src_;
  for (auto& s : streams_) {
    delete s.second;
  }
}

De10Logic& De10Logic::set_input(const Identifier* id, VId vid) {
  if (table_.find(id) == table_.end()) {
    table_.insert(id);
  }
  if (inputs_.size() <= vid) {
    inputs_.resize(vid+1, nullptr);
  }
  inputs_[vid] = id;
  return *this;
}

De10Logic& De10Logic::set_state(const Identifier* id, VId vid) {
  if (table_.find(id) == table_.end()) {
    table_.insert(id);
  }   
  state_.insert(make_pair(vid, id));
  return *this;
}

De10Logic& De10Logic::set_output(const Identifier* id, VId vid) {
  if (table_.find(id) == table_.end()) { 
    table_.insert(id);
  }
  outputs_.push_back(make_pair(id, vid));
  return *this;
}

De10Logic& De10Logic::index_tasks() {
  Inserter i(this);
  src_->accept(&i);
  return *this;
}

const VarTable32& De10Logic::get_table() const {
  return table_;
}

size_t De10Logic::num_sys_tasks() const {
  return sys_tasks_.size();
}

size_t De10Logic::num_io_tasks() const {
  return io_tasks_.size();
}

State* De10Logic::get_state() {
  auto* s = new State();
  for (const auto& sv : state_) {
    table_.read_variable(sv.second);
    s->insert(sv.first, Evaluate().get_array_value(sv.second));
  }
  return s;
}

void De10Logic::set_state(const State* s) {
  for (const auto& sv : state_) {
    const auto itr = s->find(sv.first);
    if (itr != s->end()) {
      table_.write_variable(sv.second, itr->second);
    }
  }
  table_.write_control_variable(table_.drop_update_index(), 1);
  table_.write_control_variable(table_.reset_index(), 1);
  table_.write_control_variable(table_.io_task_index(), 1);
  table_.write_control_variable(table_.sys_task_index(), 1);
}

Input* De10Logic::get_input() {
  auto* i = new Input();
  for (size_t v = 0, ve = inputs_.size(); v < ve; ++v) {
    const auto* id = inputs_[v];
    if (id == nullptr) {
      continue;
    }
    table_.read_variable(id);
    i->insert(v, Evaluate().get_value(id));
  }
  return i;
}

void De10Logic::set_input(const Input* i) {
  for (size_t v = 0, ve = inputs_.size(); v < ve; ++v) {
    const auto* id = inputs_[v];
    if (id == nullptr) {
      continue;
    }
    const auto itr = i->find(v);
    if (itr != i->end()) {
      table_.write_variable(id, itr->second);
    }
  }
  table_.write_control_variable(table_.drop_update_index(), 1);
  table_.write_control_variable(table_.reset_index(), 1);
  table_.write_control_variable(table_.io_task_index(), 1);
  table_.write_control_variable(table_.sys_task_index(), 1);
}

void De10Logic::finalize() {
  // Does nothing.
}

void De10Logic::read(VId id, const Bits* b) {
  assert(id < inputs_.size());
  assert(inputs_[id] != nullptr);
  table_.write_variable(inputs_[id], *b);
}

void De10Logic::evaluate() {
  // Read outputs and handle tasks
  wait_until_done();
  handle_outputs();
  handle_sys_tasks();
}

bool De10Logic::there_are_updates() const {
  // Read there_are_updates flag
  return table_.read_control_variable(table_.there_are_updates_index()) != 0;
}

void De10Logic::update() {
  // Throw the update trigger
  table_.write_control_variable(table_.apply_update_index(), 1);
  // Read outputs and handle tasks
  wait_until_done();
  handle_outputs();
  handle_sys_tasks();
}

bool De10Logic::there_were_tasks() const {
  return there_were_tasks_;
}

size_t De10Logic::open_loop(VId clk, bool val, size_t itr) {
  // If there are io tasks we're not going to stay in open loop for
  // very long, so just fall back to the default implementation which
  // is based on calls to evaluate and update.
  if (!io_tasks_.empty()) {
    return Core::open_loop(clk, val, itr);
  }

  // The fpga already knows the value of clk. We can ignore it.
  (void) clk;
  (void) val;

  // Go into open loop mode and handle tasks when we return. No need
  // to handle outputs. This methods assumes that we don't have any.
  table_.write_control_variable(table_.open_loop_index(), itr);
  handle_sys_tasks();

  // Return the number of iterations that we ran for
  return table_.read_control_variable(table_.open_loop_index());
}

void De10Logic::cleanup(CoreCompiler* cc) {
  auto* dc = dynamic_cast<De10Compiler*>(cc);
  assert(dc != nullptr);
  dc->cleanup(id_);
}

bool De10Logic::open_loop_enabled() const {
  ModuleInfo info(src_);
  if (info.inputs().size() != 1) {
    return false;
  }
  if (Evaluate().get_width(*info.inputs().begin()) != 1) {
    return false;
  }
  if (!info.outputs().empty()) {
    return false;
  }
  return true;
}

const Identifier* De10Logic::open_loop_clock() const {
  return open_loop_enabled() ? *ModuleInfo(src_).inputs().begin() : nullptr;
}

interfacestream* De10Logic::get_stream(FId fd) {
  const auto itr = streams_.find(fd);
  if (itr != streams_.end()) {
    return itr->second;
  }
  auto* is = new interfacestream(interface(), fd);
  streams_[fd] = is;
  return is;
}

void De10Logic::wait_until_done() {
  while (!table_.read_control_variable(table_.done_index())) {
    handle_io_tasks();
    table_.write_control_variable(table_.resume_index(), 1);
  }
  handle_io_tasks();
}

void De10Logic::handle_outputs() {
  for (const auto& o : outputs_) {
    table_.read_variable(o.first);
    interface()->write(o.second, &Evaluate().get_value(o.first));
  }
}

void De10Logic::handle_io_tasks() {
  volatile auto queue = table_.read_control_variable(table_.io_task_index());
  if (queue == 0) {
    return;
  }

  Evaluate eval;
  Sync sync(this);
  for (size_t i = 0; queue != 0; queue >>= 1, ++i) {
    if ((queue & 0x1) == 0) {
      continue;
    }
    switch (io_tasks_[i]->get_tag()) {
      case Node::Tag::fflush_statement: {
        const auto* fs = static_cast<const FflushStatement*>(io_tasks_[i]);
        fs->accept_fd(&sync);

        const auto fd = eval.get_value(fs->get_fd()).to_int();
        auto* is = get_stream(fd);
        is->flush();
        break;
      }
      case Node::Tag::fseek_statement: {
        const auto* fs = static_cast<const FseekStatement*>(io_tasks_[i]);
        fs->accept_fd(&sync);

        const auto fd = eval.get_value(fs->get_fd()).to_int();
        auto* is = get_stream(fd);

        const auto offset = eval.get_value(fs->get_offset()).to_int();
        const auto op = eval.get_value(fs->get_op()).to_int();
        const auto way = (op == 0) ? ios_base::beg : (op == 1) ? ios_base::cur : ios_base::end;

        is->clear();
        is->seekg(offset, way); 
        is->seekp(offset, way); 

        // TODO(eschkufz) update eofs
        break;
      }
      case Node::Tag::get_statement: {
        const auto* gs = static_cast<const GetStatement*>(io_tasks_[i]);
        gs->accept_fd(&sync);

        const auto fd = eval.get_value(gs->get_fd()).to_int();
        auto* is = get_stream(fd);
        Scanf().read(*is, &eval, gs);

        // TODO(eschkufz) write get back to fpga
        // TODO(eschkufz) update eofs
        break;
      }  
      case Node::Tag::put_statement: {
        const auto* ps = static_cast<const PutStatement*>(io_tasks_[i]);
        ps->accept_fd(&sync);
        ps->accept_expr(&sync);

        const auto fd = eval.get_value(ps->get_fd()).to_int();
        auto* is = get_stream(fd);
        Printf().write(*is, &eval, ps);

        // TODO(eschkufz) update eofs
        break;
      }
      default:
        assert(false);
        break;
    }
  }

  // Reset the task mask
  table_.write_control_variable(table_.io_task_index(), 0);
}

void De10Logic::handle_sys_tasks() {
  // By default, we'll assume there were no tasks
  there_were_tasks_ = false;
  volatile auto queue = table_.read_control_variable(table_.sys_task_index());
  if (queue == 0) {
    return;
  }

  // There were tasks after all; we need to empty the queue
  Evaluate eval;
  Sync sync(this);
  there_were_tasks_ = true;
  for (size_t i = 0; queue != 0; queue >>= 1, ++i) {
    if ((queue & 0x1) == 0) {
      continue;
    }
    switch (sys_tasks_[i]->get_tag()) {
      case Node::Tag::finish_statement: {
        const auto* fs = static_cast<const FinishStatement*>(sys_tasks_[i]);
        fs->accept_arg(&sync);
        interface()->finish(Evaluate().get_value(fs->get_arg()).to_int());
        break;
      }
      case Node::Tag::restart_statement: {
        const auto* rs = static_cast<const RestartStatement*>(sys_tasks_[i]);
        interface()->restart(rs->get_arg()->get_readable_val());
        break;
      }
      case Node::Tag::retarget_statement: {
        const auto* rs = static_cast<const RetargetStatement*>(sys_tasks_[i]);
        interface()->retarget(rs->get_arg()->get_readable_val());
        break;
      }
      case Node::Tag::save_statement: {
        const auto* ss = static_cast<const SaveStatement*>(sys_tasks_[i]);
        interface()->save(ss->get_arg()->get_readable_val());
        break;
      }
      default:
        assert(false);
        break;
    }
  }

  // Reset the task mask
  table_.write_control_variable(table_.sys_task_index(), 0);
}

De10Logic::Inserter::Inserter(De10Logic* de) : Visitor() {
  de_ = de;
  in_args_ = false;
}

void De10Logic::Inserter::visit(const Identifier* id) {
  if (in_args_ && (de_->table_.find(id) == de_->table_.end())) {
    de_->table_.insert(id);
  }
}

void De10Logic::Inserter::visit(const FeofExpression* fe) {
  de_->eofs_.push_back(fe);
  // TODO(eschkufz) The variable table component of this is broken now that fds
  // can be expressions
}

void De10Logic::Inserter::visit(const FinishStatement* fs) {
  de_->sys_tasks_.push_back(fs);
  in_args_ = true;
  fs->accept_arg(this);
  in_args_ = false;
}

void De10Logic::Inserter::visit(const FseekStatement* fs) {
  de_->io_tasks_.push_back(fs);
  in_args_ = true;
  fs->accept_fd(this);
  in_args_ = false;
}

void De10Logic::Inserter::visit(const GetStatement* gs) {
  de_->io_tasks_.push_back(gs);
  in_args_ = true;
  gs->accept_fd(this);
  in_args_ = false;
}

void De10Logic::Inserter::visit(const PutStatement* ps) {
  de_->io_tasks_.push_back(ps);
  in_args_ = true;
  ps->accept_fd(this);
  ps->accept_expr(this);
  in_args_ = false;
}

void De10Logic::Inserter::visit(const RestartStatement* rs) {
  de_->sys_tasks_.push_back(rs);
  in_args_ = true;
  rs->accept_arg(this);
  in_args_ = false;
}

void De10Logic::Inserter::visit(const RetargetStatement* rs) {
  de_->sys_tasks_.push_back(rs);
  in_args_ = true;
  rs->accept_arg(this);
  in_args_ = false;
}

void De10Logic::Inserter::visit(const SaveStatement* ss) {
  de_->sys_tasks_.push_back(ss);
  in_args_ = true;
  ss->accept_arg(this);
  in_args_ = false;
}

De10Logic::Sync::Sync(De10Logic* de) : Visitor() {
  de_ = de;
}

void De10Logic::Sync::visit(const Identifier* id) {
  assert(de_->table_.find(id) != de_->table_.end());
  de_->table_.read_variable(id);
}

} // namespace cascade
