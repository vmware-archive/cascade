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
#include "runtime/ids.h"
#include "target/core/stub/stub_core.h"
#include "target/core/sw/sw_clock.h"
#include "target/core.h"
#include "target/core_compiler.h"
#include "target/interface/stub/stub_interface.h"
#include "target/interface.h"
#include "target/interface_compiler.h"
#include "target/state.h"

namespace cascade {

class Engine {
  public:
    // Constructors:
    Engine();
    Engine(Interface* i, Core* c, InterfaceCompiler* ic, CoreCompiler* cc);
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
    void finalize();

    // Extended State Management Interface:
    VId get_clock_id() const;
    bool get_clock_val();
    void set_clock_val(bool t);

    // Compiler Interface:
    void replace_with(Engine* e);

  private:
    Interface* i_;
    Core* c_;
    InterfaceCompiler* ic_;
    CoreCompiler* cc_;

    bool there_are_reads_;
};

inline Engine::Engine() {
  i_ = new StubInterface();
  c_ = new StubCore(i_);
  ic_ = nullptr;
  cc_ = nullptr;
  there_are_reads_ = false;
}

inline Engine::Engine(Interface* i, Core* c, InterfaceCompiler* ic, CoreCompiler* cc) {
  assert(i != nullptr);
  assert(c != nullptr);
  i_ = i;
  c_ = c;
  ic_ = ic;
  cc_ = cc;
  there_are_reads_ = false;
}

inline Engine::~Engine() {
  if ((c_ != nullptr) && (cc_ != nullptr)) {
    c_->cleanup(cc_);
  }
  if ((i_ != nullptr) && (ic_ != nullptr)) {
    i_->cleanup(ic_);
  }
  if (c_ != nullptr) {
    delete c_;
  }
  if (i_ != nullptr) {
    delete i_;
  }
}

inline bool Engine::is_clock() const {
  return c_->is_clock();
}

inline bool Engine::is_logic() const {
  return c_->is_logic();
}

inline bool Engine::is_stub() const {
  return c_->is_stub();
}

inline bool Engine::overrides_done_step() const {
  return c_->overrides_done_step();
}

inline void Engine::done_step() {
  c_->done_step();
}

inline bool Engine::overrides_done_simulation() const {
  return c_->overrides_done_simulation();
}

inline void Engine::done_simulation() {
  c_->done_simulation();
}

inline bool Engine::there_are_reads() const {
  return there_are_reads_;
}

inline void Engine::evaluate() {
  c_->evaluate();
  there_are_reads_ = false;
}

inline bool Engine::there_are_updates() const {
  return c_->there_are_updates();
}

inline void Engine::update() {
  c_->update();
  there_are_reads_ = false;
}

inline bool Engine::there_were_tasks() const {
  return c_->there_were_tasks();
}

inline bool Engine::conditional_evaluate() {
  if (there_are_reads_) {
    evaluate();
    return true;
  }
  return false;
}

inline bool Engine::conditional_update() {
  return c_->conditional_update();
}

inline size_t Engine::open_loop(VId clk, bool val, size_t itr) {
  return c_->open_loop(clk, val, itr);
}

inline void Engine::read(VId id, const Bits* b) {
  c_->read(id, b);
  there_are_reads_ = true;
}

inline State* Engine::get_state() {
  return c_->get_state();
}

inline void Engine::set_state(const State* s) {
  c_->set_state(s);
}

inline Input* Engine::get_input() {
  return c_->get_input();
}

inline void Engine::set_input(const Input* i) {
  c_->set_input(i);
}

inline void Engine::finalize() {
  c_->finalize();
}

inline VId Engine::get_clock_id() const {
  auto* c = dynamic_cast<SwClock*>(c_);
  assert(c != nullptr);
  return c->get_id();
}

inline bool Engine::get_clock_val() {
  auto* c = dynamic_cast<SwClock*>(c_);
  assert(c != nullptr);
  return c->get_val();
}

inline void Engine::set_clock_val(bool v) {
  auto* c = dynamic_cast<SwClock*>(c_);
  assert(c != nullptr);
  c->set_val(v);
}

inline void Engine::replace_with(Engine* e) {
  // Move state and inputs from this engine into the new engine
  const auto* s = c_->get_state();
  e->c_->set_state(s);
  delete s;
  const auto* i = c_->get_input();
  e->c_->set_input(i);
  delete i;
  e->c_->finalize();

  // Now that we're done with our core and interface, clean them up
  // and delete them.
  if (cc_ != nullptr) {
    c_->cleanup(cc_);
  }
  if (ic_ != nullptr) {
    i_->cleanup(ic_);
  }
  delete c_;
  delete i_;

  // Move the internal state from the new engine into this one
  c_ = e->c_;
  cc_ = e->cc_;
  i_ = e->i_;
  ic_ = e->ic_;
  there_are_reads_ = e->there_are_reads_;

  // Delete the shell which is left over
  e->i_ = nullptr;
  e->c_ = nullptr;
  e->ic_ = nullptr;
  e->cc_ = nullptr;
  delete e;
}

} // namespace cascade

#endif
