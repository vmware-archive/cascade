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
#include "src/verilog/print/text/text_printer.h"

using namespace std;

namespace cascade {

De10Logic::De10Logic(Interface* interface, ModuleDeclaration* src, uint8_t* addr) : Logic(interface), Visitor() { 
  src_ = src;
  next_index_ = 0;
  addr_ = addr;

  src_->accept(this);
}

De10Logic::~De10Logic() {
  delete src_;
}

De10Logic& De10Logic::set_input(const Identifier* id, VId vid) {
  var_map_[id] = vid;
  if (var_table_.find(id) == var_table_.end()) {
    insert(id, true);
  }
  if (vid >= inputs_.size()) {
    inputs_.resize(vid+1, nullptr);
  }
  inputs_[vid] = &var_table_[id];

  return *this;
}

De10Logic& De10Logic::set_state(const Identifier* id, VId vid) {
  var_map_[id] = vid;
  if (var_table_.find(id) == var_table_.end()) {
    insert(id, true);
  }
  return *this;
}

De10Logic& De10Logic::set_output(const Identifier* id, VId vid) {
  var_map_[id] = vid;
  if (var_table_.find(id) == var_table_.end()) {
    insert(id, ModuleInfo(src_).is_stateful(id));
  }
  
  assert(var_table_.find(id) != var_table_.end());
  outputs_.push_back(make_pair(vid, &var_table_[id]));

  return *this;
}

State* De10Logic::get_state() {
  auto s = new State();

  ModuleInfo info(src_);
  for (auto v : info.stateful()) {
    assert(var_map_.find(v) != var_map_.end());
    assert(var_table_.find(v) != var_table_.end());

    const auto vid = var_map_[v];
    auto& vinfo = var_table_[v];

    read(&vinfo);
    s->insert(vid, vinfo.val);
  }

  return s;
}

void De10Logic::set_state(const State* s) {
  ModuleInfo info(src_);
  for (auto v : info.stateful()) {
    assert(var_map_.find(v) != var_map_.end());
    assert(var_table_.find(v) != var_table_.end());

    const auto vid = var_map_[v];
    const auto& vinfo = var_table_[v];

    const auto itr = s->find(vid);
    if (itr != s->end()) {
      write(&vinfo, &itr->second);
    }
  }
}

Input* De10Logic::get_input() {
  auto i = new Input();

  ModuleInfo info(src_);
  for (auto in : info.inputs()) {
    assert(var_map_.find(in) != var_map_.end());
    assert(var_table_.find(in) != var_table_.end());

    const auto vid = var_map_[in];
    auto& vinfo = var_table_[in];

    read(&vinfo);
    i->insert(vid, vinfo.val);
  }

  return i;
}

void De10Logic::set_input(const Input* i) {
  ModuleInfo info(src_);
  for (auto in : info.inputs()) {
    assert(var_map_.find(in) != var_map_.end());
    assert(var_table_.find(in) != var_table_.end());

    const auto vid = var_map_[in];
    const auto& vinfo = var_table_[in];

    const auto itr = i->find(vid);
    if (itr != i->end()) {
      write(&vinfo, &itr->second);
    }
  }
}

void De10Logic::resync() {
  // Read the value of the task mask
  task_queue_ = DE10_READ((uint8_t*)((sys_task_idx() << 8)|(size_t)addr_));
  // Go live!
  DE10_WRITE((uint8_t*)((live_idx() << 8)|(size_t)addr_), 1);
}

void De10Logic::read(VId id, const Bits* b) {
  assert(id < inputs_.size());
  assert(inputs_[id] != nullptr);
  write(inputs_[id], b);
}

void De10Logic::evaluate() {
  // Read outputs and handle tasks
  handle_outputs();
  handle_tasks();
}

bool De10Logic::there_are_updates() const {
  // Read there_are_updates flag
  return DE10_READ((uint8_t*)((there_are_updates_idx() << 8)|(size_t)addr_));
}

void De10Logic::update() {
  // Throw the update trigger
  DE10_WRITE((uint8_t*)((update_idx() << 8)|(size_t)addr_), 1);
  // Read outputs and handle tasks
  handle_outputs();
  handle_tasks();
}

bool De10Logic::there_were_tasks() const {
  return task_queue_ > 0;
}

size_t De10Logic::open_loop(VId clk, bool val, size_t itr) {
  // The fpga already knows the value of clk. We can ignore it.
  (void) clk;
  (void) val;

  // Go into open loop mode and handle tasks when we return. No need
  // to handle outputs. This methods assumes that we don't have any.
  DE10_WRITE((uint8_t*)((open_loop_idx() << 8)|(size_t)addr_), itr);
  handle_tasks();

  // Return the number of iterations that we ran for
  return DE10_READ((uint8_t*)((open_loop_idx() << 8)|(size_t)addr_));
}

De10Logic::map_iterator De10Logic::map_find(const Identifier* id) const {
  return var_map_.find(id);
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
  tasks_.push_back(ds);
  for (auto a : *ds->get_args()) {
    if (const auto id = dynamic_cast<const Identifier*>(a)) {
      insert(id, true);
    }
  }
}

void De10Logic::visit(const FinishStatement* fs) {
  tasks_.push_back(fs);
}

void De10Logic::visit(const WriteStatement* ws) {
  tasks_.push_back(ws);
  for (auto a : *ws->get_args()) {
    if (const auto id = dynamic_cast<const Identifier*>(a)) {
      insert(id, true);
    }
  }
}

void De10Logic::insert(const Identifier* id, bool materialized) {
  assert(var_table_.find(id) == var_table_.end());

  const auto w = Evaluate().get_width(id);
  const auto words = (w + 31) / 32;
  var_table_[id] = {next_index_, words, materialized, Bits(w, 0)};

  next_index_ += words;
}

void De10Logic::read(VarInfo* vi) {
  // Move bits from fpga into local storage one word at a time
  for (size_t idx = vi->index, n = 0; n < vi->size; ++idx, ++n) {
    const auto word = DE10_READ((uint8_t*)((idx << 8)|(size_t)addr_));
    vi->val.write_word<uint32_t>(n, word);
  }
}

void De10Logic::write(const VarInfo* vi, const Bits* b) {
  // Move bits to fpga one word at a time, ignore local storage
  for (size_t idx = vi->index, n = 0; n < vi->size; ++idx, ++n) {
    DE10_WRITE((uint8_t*)((idx << 8)|(size_t)addr_), const_cast<Bits*>(b)->read_word<uint32_t>(n));
  }
}

void De10Logic::handle_outputs() {
  for (const auto& o : outputs_) {
    read(o.second);
    interface()->write(o.first, &o.second->val);
  }
}

void De10Logic::handle_tasks() {
  // If there's nothing in the task queue, we're done
  task_queue_ = DE10_READ((uint8_t*)((sys_task_idx() << 8)|(size_t)addr_));
  if (!there_were_tasks()) {
    return;
  }
  // Empty the task queue
  auto todo = task_queue_;
  for (size_t i = 0; todo != 0; todo >>= 1, ++i) {
    if ((todo & 0x1) == 0) {
      continue;
    }
    const auto task = tasks_[i];
    if (const auto ds = dynamic_cast<const DisplayStatement*>(task)) {
      interface()->display(printf(ds->get_args()));
    } else if (const auto fs = dynamic_cast<const FinishStatement*>(task)) {
      interface()->finish(fs->get_arg()->get_val().to_int());
    } else if (const auto ws = dynamic_cast<const WriteStatement*>(task)) {
      interface()->write(printf(ws->get_args()));
    } else {
      assert(false);
    }
  }
  // Reset the task mask
  if (task_queue_ != 0) {
    DE10_WRITE((uint8_t*)((sys_task_idx() << 8)|(size_t)addr_), 0);
  }
}

string De10Logic::printf(const Many<Expression>* args) {
  if (args->empty()) {
    return "";
  }

  auto a = args->begin();
  auto s = dynamic_cast<String*>(*a);

  if (s == nullptr) {
    stringstream ss;
    Number n(evaluate(*a), Number::UNBASED);
    TextPrinter(ss) << &n;
    return ss.str();
  } 

  stringstream ss;
  for (size_t i = 0, j = 0; ; i = j+2) {
    j = s->get_readable_val().find_first_of('%', i);
    TextPrinter(ss) << s->get_readable_val().substr(i, j-i);
    if (j == string::npos) {
      break;
    }

    if (++a == args->end()) {
      continue;
    }
    stringstream temp;
    switch (s->get_readable_val()[j+1]) {
      case 'b':
      case 'B': {
        Number n(evaluate(*a), Number::BIN);
        TextPrinter(temp) << &n;
        break;
      }
      case 'd':
      case 'D': {
        Number n(evaluate(*a), Number::DEC);
        TextPrinter(temp) << &n;
        break;
      }
      case 'h':
      case 'H': {
        Number n(evaluate(*a), Number::HEX);
        TextPrinter(temp) << &n;
        break;
      }
      case 'o':
      case 'O': {
        Number n(evaluate(*a), Number::OCT);
        TextPrinter(temp) << &n;
        break;
      }
      default: 
        assert(false);
    }
    ss << temp.str().substr(temp.str().find_first_of('\'')+2);
  }
  return ss.str();
}

const Bits& De10Logic::evaluate(const Expression* e) {
  if (const auto n = dynamic_cast<const Number*>(e)) {
    return Evaluate().get_value(n);
  }
  if (const auto s = dynamic_cast<const String*>(e)) {
    return Evaluate().get_value(s);
  } 
  if (const auto id = dynamic_cast<const Identifier*>(e)) {
    const auto itr = var_table_.find(id);
    assert(itr != var_table_.end());
    read(&itr->second); 
    return itr->second.val;
  } else {
    // No support for non-primary expressions.
    assert(false);
    return Evaluate().get_value(e);
  }
}

} // namespace cascade
