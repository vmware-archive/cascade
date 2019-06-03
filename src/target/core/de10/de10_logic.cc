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
#include "target/core/common/printf.h"
#include "target/core/de10/de10_compiler.h"
#include "target/core/de10/io.h"
#include "target/input.h"
#include "target/interface.h"
#include "target/state.h"
#include "verilog/ast/ast.h"
#include "verilog/analyze/evaluate.h"
#include "verilog/analyze/module_info.h"
#include "verilog/print/text/text_printer.h"

using namespace std;

// Note that this is a plus, not a logical or! We can't guarantee that addr is aligned!
#define MANGLE(addr, idx) ((volatile uint8_t*)(((idx) << 2) + (size_t)addr))

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

vector<size_t> De10Logic::VarInfo::arity() const {
  return Evaluate().get_arity(id_);
}

size_t De10Logic::VarInfo::elements() const {
  size_t res = 1;
  for (auto d : arity()) {
    res *= d;
  }
  return res;
}

size_t De10Logic::VarInfo::bit_size() const {
  // Most of the time we'll be applying this function to variable declarations.
  // But sometimes we'll want to apply it to task arguments, which may have 
  // a different context-determined width
  const auto* r = Resolve().get_resolution(id_);
  assert(r != nullptr);
  return max(Evaluate().get_width(r), Evaluate().get_width(id_));
}

size_t De10Logic::VarInfo::element_size() const {
  return (bit_size() + 31) / 32;
}

size_t De10Logic::VarInfo::entry_size() const {
  return elements() * element_size();
}

De10Logic::De10Logic(Interface* interface, QuartusServer::Id id, ModuleDeclaration* src, volatile uint8_t* addr) : Logic(interface), Visitor() { 
  id_ = id;
  src_ = src;
  addr_ = addr;
  next_index_ = 0;
}

De10Logic::~De10Logic() {
  delete src_;
  for (auto* s : streams_) {
    if (s != nullptr) {
      delete s;
    }
  }
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

De10Logic& De10Logic::index_tasks() {
  src_->accept(this);
  return *this;
}

State* De10Logic::get_state() {
  auto* s = new State();

  ModuleInfo info(src_);
  for (auto* v : info.stateful()) {
    const auto vid = var_map_.find(v);
    assert(vid != var_map_.end());
    const auto vinfo = var_table_.find(v);
    assert(vinfo != var_table_.end());

    read_array(vinfo->second);
    s->insert(vid->second, Evaluate().get_array_value(vinfo->second.id()));
  }

  return s;
}

void De10Logic::set_state(const State* s) {
  ModuleInfo info(src_);
  for (auto* v : info.stateful()) {
    // Write value directly to de10
    const auto vid = var_map_.find(v);
    assert(vid != var_map_.end());
    const auto vinfo = var_table_.find(v);
    assert(vinfo != var_table_.end());

    const auto itr = s->find(vid->second);
    if (itr != s->end()) {
      // Write value directly to device
      write_array(vinfo->second, itr->second);
      // Write stream values to sw as well, since we'll need this value when
      // resolving io tasks
      if (info.is_stream(v)) {
        Evaluate().assign_value(vinfo->first, itr->second[0]);
      }
    }
  }

  DE10_WRITE(MANGLE(addr_, drop_update_idx()), 1);
  DE10_WRITE(MANGLE(addr_, reset_idx()), 1);
  DE10_WRITE(MANGLE(addr_, io_task_idx()), 1);
  DE10_WRITE(MANGLE(addr_, sys_task_idx()), 1);
}

Input* De10Logic::get_input() {
  auto* i = new Input();

  ModuleInfo info(src_);
  for (auto* in : info.inputs()) {
    const auto vid = var_map_.find(in);
    assert(vid != var_map_.end());
    const auto vinfo = var_table_.find(in);
    assert(vinfo != var_table_.end());

    read_scalar(vinfo->second);
    i->insert(vid->second, Evaluate().get_value(vinfo->second.id()));
  }

  return i;
}

void De10Logic::set_input(const Input* i) {
  ModuleInfo info(src_);
  for (auto* in : info.inputs()) {
    const auto vid = var_map_.find(in);
    assert(vid != var_map_.end());
    const auto vinfo = var_table_.find(in);
    assert(vinfo != var_table_.end());

    const auto itr = i->find(vid->second);
    if (itr != i->end()) {
      write_scalar(vinfo->second, itr->second);
    }
  }

  DE10_WRITE(MANGLE(addr_, drop_update_idx()), 1);
  DE10_WRITE(MANGLE(addr_, reset_idx()), 1);
  DE10_WRITE(MANGLE(addr_, io_task_idx()), 1);
  DE10_WRITE(MANGLE(addr_, sys_task_idx()), 1);
}

void De10Logic::finalize() {
  // Create interface streams for every stream variable
  ModuleInfo info(src_);
  for (auto* s : info.streams()) {
    const auto sid = Evaluate().get_value(s).to_int();
    if (sid > streams_.size()) {
      streams_.resize(sid+1, nullptr);
    }
    streams_[sid] = new interfacestream(interface(), sid);
  }

  // For every io task...
  for (auto& io : io_tasks_) {
    // Look up the stream variable that this task references
    const Identifier* r = nullptr;
    switch(get<0>(io)->get_tag()) {
      case Node::Tag::get_statement:
        r = Resolve().get_resolution(static_cast<const GetStatement*>(get<0>(io))->get_id());
        break;
      case Node::Tag::put_statement:
        r = Resolve().get_resolution(static_cast<const PutStatement*>(get<0>(io))->get_id());
        break;
      case Node::Tag::seek_statement:
        r = Resolve().get_resolution(static_cast<const SeekStatement*>(get<0>(io))->get_id());
        break;
      default:
        assert(false);
        break;
    }
    assert(r != nullptr);

    // Now that we've set state, we can use it's value to store a pointer to its underlying
    // stream in the io_task index.
    assert(Evaluate().get_value(r).to_int() < streams_.size());
    get<1>(io) = streams_[Evaluate().get_value(r).to_int()];

    // Also look up the set of eof checks that depend on this stream variable. Store the
    // var info for those check in the index as well, so we can quickly(*ish*) update those
    // values whenever we interact with this stream
    const auto itr = eof_checks_.find(r);
    assert(itr != eof_checks_.end());
    for (auto* i : itr->second) {
      const auto titr = table_find(i);
      assert(titr != table_end());
      get<2>(io).push_back(titr->second);
    }

    // And while we're here, set the initial values for those checks
    const auto val = get<1>(io)->eof();
    for (auto& vinfo : get<2>(io)) {
      write_scalar(vinfo, Bits(val));
    }
  }
}

void De10Logic::read(VId id, const Bits* b) {
  assert(id < inputs_.size());
  assert(inputs_[id] != nullptr);
  write_scalar(*inputs_[id], *b);
}

void De10Logic::evaluate() {
  // Read outputs and handle tasks
  wait_until_done();
  handle_outputs();
  handle_sys_tasks();
}

bool De10Logic::there_are_updates() const {
  // Read there_are_updates flag
  return DE10_READ(MANGLE(addr_, there_are_updates_idx()));
}

void De10Logic::update() {
  // Throw the update trigger
  DE10_WRITE(MANGLE(addr_, apply_update_idx()), 1);
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
  DE10_WRITE(MANGLE(addr_, open_loop_idx()), itr);
  handle_sys_tasks();

  // Return the number of iterations that we ran for
  return DE10_READ(MANGLE(addr_, open_loop_idx()));
}

void De10Logic::cleanup(CoreCompiler* cc) {
  auto* dc = dynamic_cast<De10Compiler*>(cc);
  assert(dc != nullptr);
  dc->cleanup(id_);
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

size_t De10Logic::there_are_updates_idx() const {
  return next_index_;
}

size_t De10Logic::apply_update_idx() const {
  return next_index_ + 1;
}
  
size_t De10Logic::drop_update_idx() const {
  return next_index_ + 2;
}
  
size_t De10Logic::sys_task_idx() const {
  return next_index_ + 3;
}

size_t De10Logic::io_task_idx() const {
  return next_index_ + 4;
}

size_t De10Logic::resume_idx() const {
  return next_index_ + 5;
}

size_t De10Logic::reset_idx() const {
  return next_index_ + 6;
}

size_t De10Logic::done_idx() const {
  return next_index_ + 7;
}

size_t De10Logic::open_loop_idx() const {
  return next_index_ + 8;
}

size_t De10Logic::sys_task_size() const {
  return sys_tasks_.size();
}
  
size_t De10Logic::io_task_size() const {
  return io_tasks_.size();
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

void De10Logic::visit(const FeofExpression* fe) {
  // This isn't technically an io task, but it needs to be associated with io tasks
  // that affect it.
  const auto* s = Resolve().get_resolution(fe->get_arg());
  assert(s != nullptr);
  eof_checks_[s].push_back(fe->get_arg());

  // Insert a materialized instance of its stream arg into the variable table.
  // It's a little bit of a hack to do this, but we only need a single bit, and
  // we know we've got it here in the AST. 
  Inserter i(this);
  fe->accept_arg(&i); 
}

void De10Logic::visit(const DisplayStatement* ds) {
  // Record this task and insert materialized instances of the
  // variables in its arguments into the variable table.
  sys_tasks_.push_back(ds);
  Inserter i(this);
  ds->accept_args(&i);
}

void De10Logic::visit(const ErrorStatement* es) {
  // Record this task and insert materialized instances of the
  // variables in its arguments into the variable table.
  sys_tasks_.push_back(es);
  Inserter i(this);
  es->accept_args(&i);
}

void De10Logic::visit(const FinishStatement* fs) {
  // Record this task and insert materialized instances of the
  // variables in its arguments into the variable table.
  sys_tasks_.push_back(fs);
  Inserter i(this);
  fs->accept_arg(&i);
}

void De10Logic::visit(const GetStatement* gs) {
  // Record this io task, but no need to descend on its argument.
  // We'll write its target directly when he handle it.
  io_tasks_.push_back(make_tuple(gs, nullptr, vector<VarInfo>()));
}

void De10Logic::visit(const InfoStatement* is) {
  // Record this task and insert materialized instances of the
  // variables in its arguments into the variable table.
  sys_tasks_.push_back(is);
  Inserter i(this);
  is->accept_args(&i);
}

void De10Logic::visit(const PutStatement* ps) {
  // Record this io task and insert a materialized instance of its argument in
  // the variable table.
  io_tasks_.push_back(make_tuple(ps, nullptr, vector<VarInfo>()));
  Inserter i(this);
  ps->accept_var(&i); 
}

void De10Logic::visit(const RetargetStatement* rs) {
  // Record this task, but no need to descend on its argument (which is a constant)
  sys_tasks_.push_back(rs);
}

void De10Logic::visit(const SeekStatement* ss) {
  // Record this task, but no need to descend on its arguments (which are constants)
  io_tasks_.push_back(make_tuple(ss, nullptr, vector<VarInfo>()));
}

void De10Logic::visit(const WarningStatement* ws) {
  // Record this task and insert materialized instances of the
  // variables in its arguments into the variable table.
  sys_tasks_.push_back(ws);
  Inserter i(this);
  ws->accept_args(&i);
}

void De10Logic::visit(const WriteStatement* ws) {
  // Record this task and insert materialized instances of the
  // variables in its arguments into the variable table.
  sys_tasks_.push_back(ws);
  Inserter i(this);
  ws->accept_args(&i);
}

void De10Logic::insert(const Identifier* id, bool materialized) {
  assert(var_table_.find(id) == var_table_.end());
  VarInfo vi(id, next_index_, materialized);
  var_table_.insert(make_pair(id, vi));

  next_index_ += vi.entry_size();
}

void De10Logic::read_scalar(const VarInfo& vi) {
  // Make sure this is actually a scalar
  assert(vi.arity().empty());
  // But use the generic array implementation
  read_array(vi);
}

void De10Logic::read_array(const VarInfo& vi) {
  // Move bits from fpga into local storage one word at a time.  Values are
  // stored in ascending order in memory, just as they are in the variable
  // table
  auto idx = vi.index();
  for (size_t i = 0, ie = vi.elements(); i < ie; ++i) {
    for (size_t j = 0, je = vi.element_size(); j < je; ++j) {
      const volatile auto word = DE10_READ(MANGLE(addr_, idx));
      Evaluate().assign_word<uint32_t>(vi.id(), i, j, word);
      ++idx;
    } 
  }
}

void De10Logic::write_scalar(const VarInfo& vi, const Bits& b) {
  // Make sure this is actually a scalar
  assert(vi.arity().empty());
  // Move bits to fpga one word at a time, skipping the local storage.
  // If an when we need a value we'll read it out of the fpga first.
  auto idx = vi.index();
  for (size_t i = 0, ie = vi.entry_size(); i < ie; ++i) {
    const volatile auto word = b.read_word<uint32_t>(i);
    DE10_WRITE(MANGLE(addr_, idx), word);
    ++idx;
  }
}

void De10Logic::write_array(const VarInfo& vi, const Vector<Bits>& bs) {
  // Move bits to fpga one word at a time, skipping the local storage.
  // If an when we need a value we'll read it out of the fpga first.
  auto idx = vi.index();
  for (size_t i = 0, ie = vi.elements(); i < ie; ++i) {
    for (size_t j = 0, je = vi.element_size(); j < je; ++j) {
      const volatile auto word = bs[i].read_word<uint32_t>(j);
      DE10_WRITE(MANGLE(addr_, idx), word);
      ++idx;
    }
  }
}

void De10Logic::wait_until_done() {
  while (!DE10_READ(MANGLE(addr_, done_idx()))) {
    handle_io_tasks();
    DE10_WRITE(MANGLE(addr_, resume_idx()), 1);
  }
  handle_io_tasks();
}

void De10Logic::handle_outputs() {
  for (const auto& o : outputs_) {
    read_scalar(*o.second);
    interface()->write(o.first, &Evaluate().get_value(o.second->id()));
  }
}

void De10Logic::handle_io_tasks() {
  volatile auto queue = DE10_READ(MANGLE(addr_, io_task_idx()));
  if (queue == 0) {
    return;
  }

  for (size_t i = 0; queue != 0; queue >>= 1, ++i) {
    if ((queue & 0x1) == 0) {
      continue;
    }
    switch (get<0>(io_tasks_[i])->get_tag()) {
      case Node::Tag::get_statement: {
        const auto* gs = static_cast<const GetStatement*>(get<0>(io_tasks_[i]));

        Bits temp;
        temp.read(*get<1>(io_tasks_[i]), 16);
        const auto itr = table_find(Resolve().get_resolution(gs->get_var()));
        assert(itr != table_end());
        write_scalar(itr->second, temp);

        const auto val = get<1>(io_tasks_[i])->eof();
        for (auto& vinfo : get<2>(io_tasks_[i])) {
          write_scalar(vinfo, Bits(val));
        }
        break;
      }  
      case Node::Tag::put_statement: {
        const auto* ps = static_cast<const PutStatement*>(get<0>(io_tasks_[i]));

        Sync sync(this);
        ps->accept_var(&sync);
        const auto& val = Evaluate().get_value(ps->get_var());
        val.write(*get<1>(io_tasks_[i]), 16);
        get<1>(io_tasks_[i])->put(' ');
        break;
      }
      case Node::Tag::seek_statement: {
        const auto* ss = static_cast<const SeekStatement*>(get<0>(io_tasks_[i]));

        const auto pos = Evaluate().get_value(ss->get_pos()).to_int();
        get<1>(io_tasks_[i])->clear();
        get<1>(io_tasks_[i])->seekg(pos);

        const auto val = get<1>(io_tasks_[i])->eof();
        for (auto& vinfo : get<2>(io_tasks_[i])) {
          write_scalar(vinfo, Bits(val));
        }
        break;
      }
      default:
        assert(false);
        break;
    }
  }

  // Reset the task mask
  DE10_WRITE(MANGLE(addr_, io_task_idx()), 0);
}

void De10Logic::handle_sys_tasks() {
  // By default, we'll assume there were no tasks
  there_were_tasks_ = false;
  volatile auto queue = DE10_READ(MANGLE(addr_, sys_task_idx()));
  if (queue == 0) {
    return;
  }

  // There were tasks after all; we need to empty the queue
  there_were_tasks_ = true;
  for (size_t i = 0; queue != 0; queue >>= 1, ++i) {
    if ((queue & 0x1) == 0) {
      continue;
    }
    Evaluate eval;
    if (sys_tasks_[i]->is(Node::Tag::display_statement)) {
      const auto* ds = static_cast<const DisplayStatement*>(sys_tasks_[i]);
      Sync sync(this);
      ds->accept_args(&sync);
      interface()->display(Printf(&eval).format(ds->begin_args(), ds->end_args()));
    } else if (sys_tasks_[i]->is(Node::Tag::error_statement)) {
      const auto* es = static_cast<const ErrorStatement*>(sys_tasks_[i]);
      Sync sync(this);
      es->accept_args(&sync);
      interface()->error(Printf(&eval).format(es->begin_args(), es->end_args()));
    } else if (sys_tasks_[i]->is(Node::Tag::finish_statement)) {
      const auto* fs = static_cast<const FinishStatement*>(sys_tasks_[i]);
      interface()->finish(Evaluate().get_value(fs->get_arg()).to_int());
    } else if (sys_tasks_[i]->is(Node::Tag::info_statement)) {
      const auto* is = static_cast<const InfoStatement*>(sys_tasks_[i]);
      Sync sync(this);
      is->accept_args(&sync);
      interface()->info(Printf(&eval).format(is->begin_args(), is->end_args()));
    } else if (sys_tasks_[i]->is(Node::Tag::restart_statement)) {
      const auto* rs = static_cast<const RestartStatement*>(sys_tasks_[i]);
      interface()->restart(rs->get_arg()->get_readable_val());
    } else if (sys_tasks_[i]->is(Node::Tag::retarget_statement)) {
      const auto* rs = static_cast<const RetargetStatement*>(sys_tasks_[i]);
      interface()->retarget(rs->get_arg()->get_readable_val());
    } else if (sys_tasks_[i]->is(Node::Tag::save_statement)) {
      const auto* ss = static_cast<const SaveStatement*>(sys_tasks_[i]);
      interface()->save(ss->get_arg()->get_readable_val());
    } else if (sys_tasks_[i]->is(Node::Tag::warning_statement)) {
      const auto* ws = static_cast<const WarningStatement*>(sys_tasks_[i]);
      Sync sync(this);
      ws->accept_args(&sync);
      interface()->warning(Printf(&eval).format(ws->begin_args(), ws->end_args()));
    } else if (sys_tasks_[i]->is(Node::Tag::write_statement)) {
      const auto* ws = static_cast<const WriteStatement*>(sys_tasks_[i]);
      Sync sync(this);
      ws->accept_args(&sync);
      interface()->write(Printf(&eval).format(ws->begin_args(), ws->end_args()));
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
  de_->read_scalar(vinfo->second);
}

} // namespace cascade
