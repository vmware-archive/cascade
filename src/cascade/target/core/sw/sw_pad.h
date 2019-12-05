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

#ifndef CASCADE_SRC_TARGET_CORE_SW_SW_PAD_H
#define CASCADE_SRC_TARGET_CORE_SW_SW_PAD_H

#include <cassert>
#include <mutex>
#include "common/bits.h"
#include "target/core.h"

namespace cascade::sw {

class SwPad : public Pad {
  public:
    SwPad(Interface* interface, VId out, size_t size, Bits* val, std::mutex* lock); 
    ~SwPad() override = default;

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
    size_t size_;
    Bits* val_;
    std::mutex* lock_;

    Bits latch_;
    bool there_are_updates_;
};

inline SwPad::SwPad(Interface* interface, VId out, size_t size, Bits* val, std::mutex* lock) : Pad(interface) {
  out_ = out;
  size_ = size;
  val_ = val;
  lock_ = lock;

  there_are_updates_ = true;
  latch_.resize(size);
}

inline State* SwPad::get_state() {
  return new State();
} 

inline void SwPad::set_state(const State* s) {
  // Stateless; does nothing
  (void) s;
}

inline Input* SwPad::get_input() {
  // Outputs only; does nothing
  return new Input();
}

inline void SwPad::set_input(const Input* i) {
  // Outputs only; does nothing
  (void) i;
}

inline bool SwPad::overrides_done_step() const {
  return true;
}

inline void SwPad::done_step() {
  there_are_updates_ = true;
}

inline void SwPad::read(VId id, const Bits* b) {
  // Invoking this method doesn't make sense
  (void) id;
  (void) b;
  assert(false);
}

inline void SwPad::evaluate() {
  interface()->write(out_, &latch_);
}

inline bool SwPad::there_are_updates() const {
  return there_are_updates_;
}

inline void SwPad::update() { 
  there_are_updates_ = false;
  {
    std::lock_guard<std::mutex> lg(*lock_);
    latch_.assign(size_-1, 0, *val_);
  }
  evaluate();
}

} // namespace cascade::sw

#endif
