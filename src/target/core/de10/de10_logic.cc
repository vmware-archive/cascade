// Copyright 2017-2018 VMware, Inc.
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

#include "src/target/core/de10/de10_logic.h"

#include <cassert>
#include "src/target/core/de10/io.h"
#include "src/target/input.h"
#include "src/target/interface.h"
#include "src/target/state.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/printf.h"
#include "src/verilog/print/text/text_printer.h"

using namespace std;

// Note that this is a plus, not a logical or! We can't guarantee that addr is aligned!
#define MANGLE(addr, idx) ((volatile uint8_t*)(((idx) << 8) + (uint32_t)addr))

namespace cascade {

De10Logic::VarInfo::VarInfo(const Identifier* id, size_t idx, bool materialized) {
  id_ = id;
  idx_ = idx;
  materialized_ = materialized;
}

const Identifier* De10Logic::VarInfo::id() const {
  return id_;
}

bool De10Logic::VarInfo::materialized() const {
  return materialized_;
}

size_t De10Logic::VarInfo::index() const {
  return idx_;
}

size_t De10Logic::VarInfo::bit_size() const {
  return Evaluate().get_width(id_);
}

size_t De10Logic::VarInfo::word_size() const {
  return (bit_size() + 31) / 32;
}

De10Logic::De10Logic(Interface* interface, ModuleDeclaration* src, volatile uint8_t* addr) : Logic(interface), Visitor() { 
  src_ = src;
  addr_ = addr;
  next_index_ = 0;
  src_->accept(this);
}

De10Logic::~De10Logic() {
  delete src_;
}

De10Logic& De10Logic::set_input(const Identifier* id, VId vid) {
  // Insert inverse mapping into the variable map
  var_map_[id] = vid;
  // Insert a materialized version of id into the variable table
  if (var_table_.find(id) == var_table_.end()) {
    insert(id, true);
  }
  // Insert a pointer to this variable table entry into the input index
  if (vid >= inputs_.size()) {
    inputs_.resize(vid+1, nullptr);
  }
  inputs_[vid] = &var_table_.find(id)->second;

  return *this;
}

De10Logic& De10Logic::set_state(const Identifier* id, VId vid) {
  // Insert inverse mapping into the variable map
  var_map_[id] = vid;
  // Insert a materialized version of id into the variable table
  if (var_table_.find(id) == var_table_.end()) {
    insert(id, true);
  }

  return *this;
}

De10Logic& De10Logic::set_output(const Identifier* id, VId vid) {
  // Insert inverse mapping into the variable map
  var_map_[id] = vid;
  // Insert a materialized version of id (but only if it's stateful) into the
  // variable table
  if (var_table_.find(id) == var_table_.end()) {
    insert(id, ModuleInfo(src_).is_stateful(id));
  }
  // Insert a pointer to this variable into the output index
  outputs_.push_back(make_pair(vid, &var_table_.find(id)->second));

  return *this;
}

State* De10Logic::get_state() {
  auto s = new State();

  ModuleInfo info(src_);
  for (auto v : info.stateful()) {
    const auto vid = var_map_.find(v);
    assert(vid != var_map_.end());
    const auto vinfo = var_table_.find(v);
    assert(vinfo != var_table_.end());

    read(vinfo->second);
    s->insert(vid->second, Evaluate().get_value(vinfo->second.id()));
  }

  return s;
}

void De10Logic::set_state(const State* s) {
  ModuleInfo info(src_);
  for (auto v : info.stateful()) {
    const auto vid = var_map_.find(v);
    assert(vid != var_map_.end());
    const auto vinfo = var_table_.find(v);
    assert(vinfo != var_table_.end());

    const auto itr = s->find(vid->second);
    if (itr != s->end()) {
      write(vinfo->second, itr->second);
    }
  }
}

Input* De10Logic::get_input() {
  auto i = new Input();

  ModuleInfo info(src_);
  for (auto in : info.inputs()) {
    const auto vid = var_map_.find(in);
    assert(vid != var_map_.end());
    const auto vinfo = var_table_.find(in);
    assert(vinfo != var_table_.end());

    read(vinfo->second);
    i->insert(vid->second, Evaluate().get_value(vinfo->second.id()));
  }

  return i;
}

void De10Logic::set_input(const Input* i) {
  ModuleInfo info(src_);
  for (auto in : info.inputs()) {
    const auto vid = var_map_.find(in);
    assert(vid != var_map_.end());
    const auto vinfo = var_table_.find(in);
    assert(vinfo != var_table_.end());

    const auto itr = i->find(vid->second);
    if (itr != i->end()) {
      write(vinfo->second, itr->second);
    }
  }
}

void De10Logic::resync() {
  // Reset the task queue and go live
  DE10_WRITE(MANGLE(addr_, sys_task_idx()), 0);
  DE10_WRITE(MANGLE(addr_, live_idx()), 1);
}

void De10Logic::read(VId id, const Bits* b) {
  assert(id < inputs_.size());
  assert(inputs_[id] != nullptr);
  write(*inputs_[id], *b);
}

void De10Logic::evaluate() {
  // Read outputs and handle tasks
  handle_outputs();
  handle_tasks();
}

bool De10Logic::there_are_updates() const {
  // Read there_are_updates flag
  return DE10_READ(MANGLE(addr_, there_are_updates_idx()));
}

void De10Logic::update() {
  // Throw the update trigger
  DE10_WRITE(MANGLE(addr_, update_idx()), 1);
  // Read outputs and handle tasks
  handle_outputs();
  handle_tasks();
}

bool De10Logic::there_were_tasks() const {
  return there_were_tasks_;
}

size_t De10Logic::open_loop(VId clk, bool val, size_t itr) {
  // The fpga already knows the value of clk. We can ignore it.
  (void) clk;
  (void) val;

  // Go into open loop mode and handle tasks when we return. No need
  // to handle outputs. This methods assumes that we don't have any.
  DE10_WRITE(MANGLE(addr_, open_loop_idx()), itr);
  handle_tasks();

  // Return the number of iterations that we ran for
  return DE10_READ(MANGLE(addr_, open_loop_idx()));
}

De10Logic::map_iterator De10Logic::map_begin() const {
  return var_map_.begin();
}

De10Logic::map_iterator De10Logic::map_end() const {
  return var_map_.end();
}

De10Logic::table_iterator De10Logic::table_find(const Identifier* id) const {
  return var_table_.find(id);
}

De10Logic::table_iterator De10Logic::table_begin() const {
  return var_table_.begin();
}

De10Logic::table_iterator De10Logic::table_end() const {
  return var_table_.end();
}

size_t De10Logic::table_size() const {
  return next_index_;
}

size_t De10Logic::live_idx() const {
  return next_index_;
}

size_t De10Logic::there_are_updates_idx() const {
  return next_index_ + 1;
}

size_t De10Logic::update_idx() const {
  return next_index_ + 2;
}
  
size_t De10Logic::sys_task_idx() const {
  return next_index_ + 3;
}

size_t De10Logic::open_loop_idx() const {
  return next_index_ + 4;
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

void De10Logic::visit(const DisplayStatement* ds) {
  // Record this task and insert materialized instances of the
  // variables in its arguments into the variable table.
  tasks_.push_back(ds);
  Inserter i(this);
  ds->get_args()->accept(&i);
}

void De10Logic::visit(const FinishStatement* fs) {
  // Record this task. The only arguments we expect to see here are numeric
  // constants, so no need to interact with the variable table.
  tasks_.push_back(fs);
}

void De10Logic::visit(const WriteStatement* ws) {
  // Record this task and insert materialized instances of the
  // variables in its arguments into the variable table.
  tasks_.push_back(ws);
  Inserter i(this);
  ws->get_args()->accept(&i);
}

void De10Logic::insert(const Identifier* id, bool materialized) {
  assert(var_table_.find(id) == var_table_.end());
  VarInfo vi(id, next_index_, materialized);
  var_table_.insert(make_pair(id, vi));

  next_index_ += vi.word_size();
}

void De10Logic::read(const VarInfo& vi) {
  // Move bits from fpga into local storage one word at a time
  for (size_t idx = vi.index(), n = 0, ne = vi.word_size(); n < ne; ++idx, ++n) {
    const volatile auto word = DE10_READ(MANGLE(addr_, idx));
    Evaluate().assign_word<uint32_t>(vi.id(), n, word);
  }
}

void De10Logic::write(const VarInfo& vi, const Bits& b) {
  // Move bits to fpga one word at a time, skipping the local storage.
  // If an when we need a value we'll read it out of the fpga first.
  for (size_t idx = vi.index(), n = 0, ne = vi.word_size(); n < ne; ++idx, ++n) {
    const volatile auto word = b.read_word<uint32_t>(n);
    DE10_WRITE(MANGLE(addr_, idx), word);
  }
}

void De10Logic::handle_outputs() {
  for (const auto& o : outputs_) {
    read(*o.second);
    interface()->write(o.first, &Evaluate().get_value(o.second->id()));
  }
}

void De10Logic::handle_tasks() {
  // By default, we'll assume there were no tasks
  there_were_tasks_ = false;
  volatile auto task_queue = DE10_READ(MANGLE(addr_, sys_task_idx()));
  if (task_queue == 0) {
    return;
  }

  // There were tasks after all; we need to empty the queue
  there_were_tasks_ = true;
  for (size_t i = 0; task_queue != 0; task_queue >>= 1, ++i) {
    if ((task_queue & 0x1) == 0) {
      continue;
    }
    if (const auto ds = dynamic_cast<const DisplayStatement*>(tasks_[i])) {
      Sync sync(this);
      ds->get_args()->accept(&sync);
      interface()->display(Printf().format(ds->get_args()));
    } else if (const auto fs = dynamic_cast<const FinishStatement*>(tasks_[i])) {
      interface()->finish(fs->get_arg()->get_val().to_int());
    } else if (const auto ws = dynamic_cast<const WriteStatement*>(tasks_[i])) {
      Sync sync(this);
      ws->get_args()->accept(&sync);
      interface()->write(Printf().format(ws->get_args()));
    } else {
      assert(false);
    }
  }

  // Reset the task mask
  DE10_WRITE(MANGLE(addr_, sys_task_idx()), 0);
}

De10Logic::Inserter::Inserter(De10Logic* de) : Visitor() {
  de_ = de;
}

void De10Logic::Inserter::visit(const Identifier* id) {
  de_->insert(id, true);
}

De10Logic::Sync::Sync(De10Logic* de) : Visitor() {
  de_ = de;
}

void De10Logic::Sync::visit(const Identifier* id) {
  const auto vinfo = de_->var_table_.find(id);
  assert(vinfo != de_->var_table_.end());
  de_->read(vinfo->second);
}

#undef MANGLE

} // namespace cascade
