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

#ifndef CASCADE_SRC_TARGET_ENGINE_H
#define CASCADE_SRC_TARGET_ENGINE_H

#include <cassert>
#include "src/runtime/ids.h"
#include "src/target/core/stub/stub_core.h"
#include "src/target/core/sw/sw_clock.h"
#include "src/target/core.h"
#include "src/target/interface/stub/stub_interface.h"
#include "src/target/interface.h"
#include "src/target/state.h"

namespace cascade {

class Engine {
  public:
    // Constructors:
    Engine();
    Engine(Core* core, Interface* interface);
    ~Engine();

    // Query Interface:
    bool is_clock() const;
    bool is_logic() const;
    bool is_stub() const;

    // Scheduling Interface:
    bool overrides_done_step() const;
    void done_step();
    bool overrides_done_simulation() const;
    void done_simulation();
    bool there_are_reads() const;
    void evaluate();
    bool there_are_updates() const;
    void update();
    bool there_were_tasks() const;

    // Optimized Scheduling Tnterface:
    bool conditional_evaluate();
    bool conditional_update();
    size_t open_loop(VId clk, bool val, size_t itr);

    // I/O Interface:
    void read(VId id, const Bits* b);

    // State Management Interface:
    State* get_state();
    void set_state(const State* s);
    Input* get_input();
    void set_input(const Input* i);
    void resync();

    // Extended State Management Interface:
    // TODO: Does this really belong here? The only place this is called is in
    // the open loop scheduler. This could be a lot more efficient if it were
    // only defined for clocks.
    bool get_bit(VId id);
    void set_bit(VId id, bool t);

    // Compiler Interface:
    void replace_with(Engine* e);

  private:
    Core* core_;
    Interface* interface_;

    bool there_are_reads_;
};

inline Engine::Engine() {
  interface_ = new StubInterface();
  core_ = new StubCore(interface_);
  there_are_reads_ = false;
}

inline Engine::Engine(Core* core, Interface* interface) {
  core_ = core;
  interface_ = interface;
  there_are_reads_ = false;
}

inline Engine::~Engine() {
  if (core_ != nullptr) {
    delete core_;
  }
  if (interface_ != nullptr) {
    delete interface_;
  }
}

inline bool Engine::is_clock() const {
  return dynamic_cast<Clock*>(core_) != nullptr;
}

inline bool Engine::is_logic() const {
  return dynamic_cast<Logic*>(core_) != nullptr;
}

inline bool Engine::is_stub() const {
  return dynamic_cast<StubCore*>(core_) != nullptr;
}

inline bool Engine::overrides_done_step() const {
  return core_->overrides_done_step();
}

inline void Engine::done_step() {
  core_->done_step();
}

inline bool Engine::overrides_done_simulation() const {
  return core_->overrides_done_simulation();
}

inline void Engine::done_simulation() {
  core_->done_simulation();
}

inline bool Engine::there_are_reads() const {
  return there_are_reads_;
}

inline void Engine::evaluate() {
  core_->evaluate();
  there_are_reads_ = false;
}

inline bool Engine::there_are_updates() const {
  return core_->there_are_updates();
}

inline void Engine::update() {
  core_->update();
  there_are_reads_ = false;
}

inline bool Engine::there_were_tasks() const {
  return core_->there_were_tasks();
}

inline bool Engine::conditional_evaluate() {
  if (there_are_reads_) {
    evaluate();
    return true;
  }
  return false;
}

inline bool Engine::conditional_update() {
  const auto res = core_->conditional_update();
  there_are_reads_ = false;
  return res;
}

inline size_t Engine::open_loop(VId clk, bool val, size_t itr) {
  return core_->open_loop(clk, val, itr);
}

inline void Engine::read(VId id, const Bits* b) {
  core_->read(id, b);
  there_are_reads_ = true;
}

inline State* Engine::get_state() {
  return core_->get_state();
}

inline void Engine::set_state(const State* s) {
  core_->set_state(s);
}

inline Input* Engine::get_input() {
  return core_->get_input();
}

inline void Engine::set_input(const Input* i) {
  core_->set_input(i);
}

inline void Engine::resync() {
  core_->resync();
}

inline bool Engine::get_bit(VId id) {
  auto s = get_state();
  assert(s->find(id) != s->end());
  assert(s->find(id)->second.size() == 1);

  const auto res = s->find(id)->second[0].to_bool();
  delete s;

  return res;
}

inline void Engine::set_bit(VId id, bool b) {
  auto s = new State();
  s->insert(id, Bits(1, b));
  set_state(s);
  delete s;
}

inline void Engine::replace_with(Engine* e) {
  const auto s = core_->get_state();
  e->core_->set_state(s);
  delete s;

  const auto i = core_->get_input();
  e->core_->set_input(i);
  delete i;

  e->core_->resync();

  delete core_;
  core_ = e->core_;
  e->core_ = nullptr;

  delete interface_;
  interface_ = e->interface_;
  e->interface_ = nullptr;

  there_are_reads_ = e->there_are_reads_;
  delete e;
}

} // namespace cascade

#endif
