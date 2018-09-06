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

#ifndef CASCADE_SRC_TARGET_CORE_SW_SW_CLOCK_H
#define CASCADE_SRC_TARGET_CORE_SW_SW_CLOCK_H

#include <cassert>
#include "src/base/bits/bits.h"
#include "src/target/core.h"
#include "src/target/input.h"
#include "src/target/interface.h"
#include "src/target/state.h"

namespace cascade {

class SwClock : public Clock {
  public:
    SwClock(Interface* interface, VId out);
    ~SwClock() override = default;

    State* get_state() override;
    void set_state(const State* s) override;
    Input* get_input() override;
    void set_input(const Input* i) override;

    bool overrides_done_step() const override;
    void done_step() override;

    void read(VId id, const Bits* b) override;
    void evaluate() override;
    bool there_are_updates() const override;
    void update() override;

  private:
    VId out_;
    bool val_; 
    bool there_are_updates_;
};

inline SwClock::SwClock(Interface* interface, VId out) : Clock(interface) { 
  assert(out != nullid());
  out_ = out;
  val_ = false;
  there_are_updates_ = true;
}

inline State* SwClock::get_state() {
  auto s = new State();
  s->insert(out_, Bits(1, val_ ? 1 : 0));
  return s;
}

inline void SwClock::set_state(const State* s) {
  const auto itr = s->find(out_);
  if (itr != s->end()) {
    val_ = itr->second.to_bool();
  }
}

inline Input* SwClock::get_input() {
  // Outputs only; does nothing
  return new Input();
}

inline void SwClock::set_input(const Input* i) {
  // Outputs only; does nothing
  (void) i;
}

inline bool SwClock::overrides_done_step() const {
  return true;
}

inline void SwClock::done_step() {
  there_are_updates_ = true;
}

inline void SwClock::read(VId id, const Bits* b) {
  // Clocks should never have an input
  assert(false);
  (void) id;
  (void) b;
}

inline void SwClock::evaluate() { 
  interface()->write(out_, val_);
}

inline bool SwClock::there_are_updates() const {
  return there_are_updates_;
}

inline void SwClock::update() {
  there_are_updates_ = false; 
  val_ = !val_;
  interface()->write(out_, val_);
}

} // namespace cascade

#endif
