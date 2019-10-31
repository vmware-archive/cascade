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

#ifndef CASCADE_SRC_TARGET_CORE_AVMM_AVMM_LOGIC_H
#define CASCADE_SRC_TARGET_CORE_AVMM_AVMM_LOGIC_H

#include <cassert>
#include <functional>
#include <unordered_map>
#include <vector>
#include "common/bits.h"
#include "target/core.h"
#include "target/core/avmm/var_table.h"
#include "target/core/common/interfacestream.h"
#include "target/core/common/printf.h"
#include "target/core/common/scanf.h"
#include "target/input.h"
#include "target/state.h"
#include "verilog/ast/ast.h"
#include "verilog/analyze/evaluate.h"
#include "verilog/analyze/module_info.h"
#include "verilog/ast/visitors/visitor.h"

#include "target/core/avmm/syncbuf.h"

namespace cascade::avmm {

class AvmmLogic : public Logic {
  public:
    // Typedefs:
    typedef std::function<void()> Callback;

    // Constructors:
    AvmmLogic(Interface* interface, ModuleDeclaration* md, syncbuf& reqs, syncbuf& resps);
    ~AvmmLogic() override;

    // Configuration Methods:
    AvmmLogic& set_input(const Identifier* id, VId vid);
    AvmmLogic& set_state(const Identifier* id, VId vid);
    AvmmLogic& set_output(const Identifier* id, VId vid);
    AvmmLogic& index_tasks();
    AvmmLogic& set_callback(Callback cb);

    // Configuraton Properties:
    const VarTable32& get_table() const;

    // Core Interface:
    State* get_state() override;
    void set_state(const State* s) override;
    Input* get_input() override;
    void set_input(const Input* i) override;
    void finalize() override;

    void read(VId id, const Bits* b) override;
    void evaluate() override;
    bool there_are_updates() const override;
    void update() override;
    bool there_were_tasks() const override;

    size_t open_loop(VId clk, bool val, size_t itr) override;

    // Optimization Properties:
    bool open_loop_enabled() const;
    const Identifier* open_loop_clock() const;

  private:
    // Compiler State:
    Callback cb_;

    // Source Management:
    ModuleDeclaration* src_;
    std::vector<const Identifier*> inputs_;
    std::unordered_map<VId, const Identifier*> state_;
    std::vector<std::pair<const Identifier*, VId>> outputs_;
    std::vector<const SystemTaskEnableStatement*> tasks_;

    // Control State:
    bool there_were_tasks_;
    VarTable32 table_;
    std::unordered_map<FId, interfacestream*> streams_;

    // Control Helpers:
    interfacestream* get_stream(FId fd);
    void update_eofs();
    void loop_until_done();
    void handle_outputs();
    void handle_tasks();

    // Indexes system tasks and inserts the identifiers which appear in those
    // tasks into the variable table.
    class Inserter : public Visitor {
      public:
        explicit Inserter(AvmmLogic* de);
        ~Inserter() override = default;
      private:
        AvmmLogic* de_;
        bool in_args_;
        void visit(const Identifier* id) override;
        void visit(const FeofExpression* fe) override;
        void visit(const DebugStatement* ds) override;
        void visit(const FflushStatement* fs) override;
        void visit(const FinishStatement* fs) override;
        void visit(const FseekStatement* fs) override;
        void visit(const GetStatement* gs) override;
        void visit(const PutStatement* ps) override;
        void visit(const RestartStatement* rs) override;
        void visit(const RetargetStatement* rs) override;
        void visit(const SaveStatement* ss) override;
    };

    // Synchronizes the locations in the variable table which correspond to the
    // identifiers which appear in an AST subtree. 
    class Sync : public Visitor {
      public:
        explicit Sync(AvmmLogic* de);
        ~Sync() override = default;
      private:
        AvmmLogic* de_;
        void visit(const Identifier* id) override;
    };
};

inline AvmmLogic::AvmmLogic(Interface* interface, ModuleDeclaration* src, syncbuf& reqs, syncbuf& resps) : Logic(interface), table_(reqs, resps) { 
  src_ = src;
  cb_ = nullptr;
  tasks_.push_back(nullptr);
}

inline AvmmLogic::~AvmmLogic() {
  if (cb_ != nullptr) {
    cb_();
  }
  delete src_;
  for (auto& s : streams_) {
    delete s.second;
  }
}

inline AvmmLogic& AvmmLogic::set_input(const Identifier* id, VId vid) {
  if (table_.var_find(id) == table_.var_end()) {
    table_.insert_var(id);
  }
  if (inputs_.size() <= vid) {
    inputs_.resize(vid+1, nullptr);
  }
  inputs_[vid] = id;
  return *this;
}

inline AvmmLogic& AvmmLogic::set_state(const Identifier* id, VId vid) {
  if (table_.var_find(id) == table_.var_end()) {
    table_.insert_var(id);
  }   
  state_.insert(std::make_pair(vid, id));
  return *this;
}

inline AvmmLogic& AvmmLogic::set_output(const Identifier* id, VId vid) {
  if (table_.var_find(id) == table_.var_end()) { 
    table_.insert_var(id);
  }
  outputs_.push_back(std::make_pair(id, vid));
  return *this;
}

inline AvmmLogic& AvmmLogic::index_tasks() {
  Inserter i(this);
  src_->accept(&i);
  return *this;
}

inline AvmmLogic& AvmmLogic::set_callback(Callback cb) {
  cb_ = cb;
  return *this;
}

inline const VarTable32& AvmmLogic::get_table() const {
  return table_;
}

inline State* AvmmLogic::get_state() {
  auto* s = new State();
  for (const auto& sv : state_) {
    table_.read_var(sv.second);
    s->insert(sv.first, Evaluate().get_array_value(sv.second));
  }
  return s;
}

inline void AvmmLogic::set_state(const State* s) {
  for (const auto& sv : state_) {
    const auto itr = s->find(sv.first);
    if (itr != s->end()) {
      table_.write_var(sv.second, itr->second);
    }
  }
  // Drop updates and reset state (continue once to clear any pending tasks
  table_.write_control_var(table_.drop_update_index(), 1);
  table_.write_control_var(table_.reset_index(), 1);
  table_.write_control_var(table_.resume_index(), 1);
}

inline Input* AvmmLogic::get_input() {
  auto* i = new Input();
  for (size_t v = 0, ve = inputs_.size(); v < ve; ++v) {
    const auto* id = inputs_[v];
    if (id == nullptr) {
      continue;
    }
    table_.read_var(id);
    i->insert(v, Evaluate().get_value(id));
  }
  return i;
}

inline void AvmmLogic::set_input(const Input* i) {
  for (size_t v = 0, ve = inputs_.size(); v < ve; ++v) {
    const auto* id = inputs_[v];
    if (id == nullptr) {
      continue;
    }
    const auto itr = i->find(v);
    if (itr != i->end()) {
      table_.write_var(id, itr->second);
    }
  }
  // Drop updates and reset state (continue once to clear any pending tasks
  table_.write_control_var(table_.drop_update_index(), 1);
  table_.write_control_var(table_.reset_index(), 1);
  table_.write_control_var(table_.resume_index(), 1);
}

inline void AvmmLogic::finalize() {
  // Does nothing.
}

inline void AvmmLogic::read(VId id, const Bits* b) {
  assert(id < inputs_.size());
  assert(inputs_[id] != nullptr);
  table_.write_var(inputs_[id], *b);
}

inline void AvmmLogic::evaluate() {
  // Read outputs and handle tasks
  loop_until_done();
  handle_outputs();
}

inline bool AvmmLogic::there_are_updates() const {
  // Read there_are_updates flag
  return table_.read_control_var(table_.there_are_updates_index()) != 0;
}

inline void AvmmLogic::update() {
  // Throw the update trigger
  table_.write_control_var(table_.apply_update_index(), 1);
  // Read outputs and handle tasks
  loop_until_done();
  handle_outputs();
}

inline bool AvmmLogic::there_were_tasks() const {
  return there_were_tasks_;
}

inline size_t AvmmLogic::open_loop(VId clk, bool val, size_t itr) {
  // The fpga already knows the value of clk. We can ignore it.
  (void) clk;
  (void) val;

  // Reset the tasks flag and go into open loop mode. If we exit in a done
  // state, then we performed every iteration.
  there_were_tasks_ = false;
  table_.write_control_var(table_.open_loop_index(), itr);
  if (table_.read_control_var(table_.done_index())) {
    assert(table_.read_control_var(table_.open_loop_index()) == 0);
    return itr;
  }
  // Otherwise, check how many iterations we made it through, reset the open
  // loop counter, and finish evaluating this iteration.
  const size_t counter = table_.read_control_var(table_.open_loop_index());
  table_.write_control_var(table_.open_loop_index(), 0);
  while (!table_.read_control_var(table_.done_index())) {
    handle_tasks();
    table_.write_control_var(table_.resume_index(), 1);
  }
  // No need to handle outputs here (or above). This optimization assumes that
  // this module doesn't have any.
  return itr - counter;
}

inline bool AvmmLogic::open_loop_enabled() const {
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

inline const Identifier* AvmmLogic::open_loop_clock() const {
  return open_loop_enabled() ? *ModuleInfo(src_).inputs().begin() : nullptr;
}

inline interfacestream* AvmmLogic::get_stream(FId fd) {
  const auto itr = streams_.find(fd);
  if (itr != streams_.end()) {
    return itr->second;
  }
  auto* is = new interfacestream(interface(), fd);
  streams_[fd] = is;
  return is;
}

inline void AvmmLogic::update_eofs() {
  // TODO(eschkufz): The correctness of this method relies on two invariants:
  // 
  // 1. The only expressions in the expression segment of the variable table
  //    are feofs (this is a property of our code and tru by current construction)
  // 2. feofs only refer to streams by static constants. This is a property of
  //    the program and we don't currently verify it.

  Evaluate eval;
  Sync sync(this);
  for (auto i = table_.expr_begin(), ie = table_.expr_end(); i != ie; ++i) {
    assert(i->first->is(Node::Tag::feof_expression));
    const auto* fe = static_cast<const FeofExpression*>(i->first);
    fe->accept_fd(&sync);

    auto* is = get_stream(eval.get_value(fe->get_fd()).to_uint());
    table_.write_expr(i->first, is->eof() ? 1 : 0);
  }
}

inline void AvmmLogic::loop_until_done() {
  // This method is invokved from evaluate, update, and open_loop.  The only
  // thing that prevents done from being true is the occurrence of a system
  // task. If this happens, continue asserting __continue (resume_index) and
  // handling tasks until done returns true.
  for (there_were_tasks_ = false; !table_.read_control_var(table_.done_index()); ) {
    handle_tasks();
    table_.write_control_var(table_.resume_index(), 1);
  }
}

inline void AvmmLogic::handle_outputs() {
  for (const auto& o : outputs_) {
    table_.read_var(o.first);
    interface()->write(o.second, &Evaluate().get_value(o.first));
  }
}

inline void AvmmLogic::handle_tasks() {
  // TODO(eschkufz) we'll need to support multiple task ids and iterate over all of them

  volatile auto task_id = table_.read_control_var(table_.there_were_tasks_index());
  if (task_id == 0) {
    return;
  }
  const auto* task = tasks_[task_id];

  Evaluate eval;
  Sync sync(this);
  switch (task->get_tag()) {
    case Node::Tag::debug_statement: {
      const auto* ds = static_cast<const DebugStatement*>(task);
      std::stringstream ss;
      ss << ds->get_arg();
      interface()->debug(Evaluate().get_value(ds->get_action()).to_uint(), ss.str());
      break;
    }
    case Node::Tag::finish_statement: {
      const auto* fs = static_cast<const FinishStatement*>(task);
      fs->accept_arg(&sync);
      interface()->finish(eval.get_value(fs->get_arg()).to_uint());
      there_were_tasks_ = true;
      break;
    }
    case Node::Tag::fflush_statement: {
      const auto* fs = static_cast<const FflushStatement*>(task);
      fs->accept_fd(&sync);

      const auto fd = eval.get_value(fs->get_fd()).to_uint();
      auto* is = get_stream(fd);
      is->clear();
      is->flush();
      update_eofs();

      break;
    }
    case Node::Tag::fseek_statement: {
      const auto* fs = static_cast<const FseekStatement*>(task);
      fs->accept_fd(&sync);

      const auto fd = eval.get_value(fs->get_fd()).to_uint();
      auto* is = get_stream(fd);

      const auto offset = eval.get_value(fs->get_offset()).to_uint();
      const auto op = eval.get_value(fs->get_op()).to_uint();
      const auto way = (op == 0) ? std::ios_base::beg : (op == 1) ? std::ios_base::cur : std::ios_base::end;

      is->clear();
      is->seekg(offset, way); 
      is->seekp(offset, way); 
      update_eofs();

      break;
    }
    case Node::Tag::get_statement: {
      const auto* gs = static_cast<const GetStatement*>(task);
      gs->accept_fd(&sync);

      const auto fd = eval.get_value(gs->get_fd()).to_uint();
      auto* is = get_stream(fd);
      Scanf().read(*is, &eval, gs);

      if (gs->is_non_null_var()) {
        const auto* r = Resolve().get_resolution(gs->get_var());
        assert(r != nullptr);
        table_.write_var(r, eval.get_value(r));
      }
      update_eofs();

      break;
    }  
    case Node::Tag::put_statement: {
      const auto* ps = static_cast<const PutStatement*>(task);
      ps->accept_fd(&sync);
      ps->accept_expr(&sync);

      const auto fd = eval.get_value(ps->get_fd()).to_uint();
      auto* is = get_stream(fd);
      Printf().write(*is, &eval, ps);
      update_eofs();

      break;
    }
    case Node::Tag::restart_statement: {
      const auto* rs = static_cast<const RestartStatement*>(task);
      interface()->restart(rs->get_arg()->get_readable_val());
      there_were_tasks_ = true;
      break;
    }
    case Node::Tag::retarget_statement: {
      const auto* rs = static_cast<const RetargetStatement*>(task);
      interface()->retarget(rs->get_arg()->get_readable_val());
      there_were_tasks_ = true;
      break;
    }
    case Node::Tag::save_statement: {
      const auto* ss = static_cast<const SaveStatement*>(task);
      interface()->save(ss->get_arg()->get_readable_val());
      there_were_tasks_ = true;
      break;
    }
    default:
      assert(false);
      break;
  }
}

inline AvmmLogic::Inserter::Inserter(AvmmLogic* de) : Visitor() {
  de_ = de;
  in_args_ = false;
}

inline void AvmmLogic::Inserter::visit(const Identifier* id) {
  Visitor::visit(id);
  const auto* r = Resolve().get_resolution(id);
  if (in_args_ && (de_->table_.var_find(r) == de_->table_.var_end())) {
    de_->table_.insert_var(r);
  }
}

inline void AvmmLogic::Inserter::visit(const FeofExpression* fe) {
  // We need *both* an image of this expression in the variable table to
  // represent its value, *and also* images of its arguments so that we can
  // inspect an feof expression and know what stream it refers to.

  de_->table_.insert_expr(fe); 
  in_args_ = true;
  fe->accept_fd(this);
  in_args_ = false;
}

inline void AvmmLogic::Inserter::visit(const DebugStatement* ds) {
  de_->tasks_.push_back(ds);
  // Don't descend, there aren't any expressions below here
}

inline void AvmmLogic::Inserter::visit(const FflushStatement* fs) {
  de_->tasks_.push_back(fs);
  in_args_ = true;
  fs->accept_fd(this);
  in_args_ = false;
}

inline void AvmmLogic::Inserter::visit(const FinishStatement* fs) {
  de_->tasks_.push_back(fs);
  in_args_ = true;
  fs->accept_arg(this);
  in_args_ = false;
}

inline void AvmmLogic::Inserter::visit(const FseekStatement* fs) {
  de_->tasks_.push_back(fs);
  in_args_ = true;
  fs->accept_fd(this);
  in_args_ = false;
}

inline void AvmmLogic::Inserter::visit(const GetStatement* gs) {
  de_->tasks_.push_back(gs);
  in_args_ = true;
  gs->accept_fd(this);
  in_args_ = false;
}

inline void AvmmLogic::Inserter::visit(const PutStatement* ps) {
  de_->tasks_.push_back(ps);
  in_args_ = true;
  ps->accept_fd(this);
  ps->accept_expr(this);
  in_args_ = false;
}

inline void AvmmLogic::Inserter::visit(const RestartStatement* rs) {
  de_->tasks_.push_back(rs);
  in_args_ = true;
  rs->accept_arg(this);
  in_args_ = false;
}

inline void AvmmLogic::Inserter::visit(const RetargetStatement* rs) {
  de_->tasks_.push_back(rs);
  in_args_ = true;
  rs->accept_arg(this);
  in_args_ = false;
}

inline void AvmmLogic::Inserter::visit(const SaveStatement* ss) {
  de_->tasks_.push_back(ss);
  in_args_ = true;
  ss->accept_arg(this);
  in_args_ = false;
}

inline AvmmLogic::Sync::Sync(AvmmLogic* de) : Visitor() {
  de_ = de;
}

inline void AvmmLogic::Sync::visit(const Identifier* id) {
  Visitor::visit(id);
  const auto* r = Resolve().get_resolution(id);
  assert(r != nullptr);
  assert(de_->table_.var_find(r) != de_->table_.var_end());
  de_->table_.read_var(r);
}

} // namespace cascade::avmm

#endif
