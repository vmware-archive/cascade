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

#ifndef CASCADE_SRC_TARGET_CORE_SW_SW_LED_H
#define CASCADE_SRC_TARGET_CORE_SW_SW_LED_H

#include <mutex>
#include "common/bits.h"
#include "target/core.h"

namespace cascade::sw {

class SwLed : public Led {
  public:
    SwLed(Interface* interface, VId in, size_t size, Bits* val, std::mutex* lock);
    ~SwLed() override = default;

    State* get_state() override;
    void set_state(const State* s) override;
    Input* get_input() override;
    void set_input(const Input* i) override;

    void read(VId id, const Bits* b) override;
    void evaluate() override;
    bool there_are_updates() const override;
    void update() override;

  private:
    VId in_;
    size_t size_;
    Bits* val_;
    std::mutex* lock_;
};

inline SwLed::SwLed(Interface* interface, VId in, size_t size, Bits* val, std::mutex* lock) : Led(interface) {
  in_ = in;
  size_ = size;
  val_ = val;
  lock_ = lock;
}

inline State* SwLed::get_state() {
  return new State();
} 

inline void SwLed::set_state(const State* s) {
  // Stateless; does nothing
  (void) s;
}

inline Input* SwLed::get_input() {
  std::lock_guard<std::mutex> lg(*lock_);
  auto* i = new Input();
  i->insert(in_, *val_);
  return i;
}

inline void SwLed::set_input(const Input* i) {
  std::lock_guard<std::mutex> lg(*lock_);
  const auto itr = i->find(in_);
  if (itr != i->end()) {
    val_->assign(itr->second);
  }
}

inline void SwLed::read(VId id, const Bits* b) {
  (void) id;
  std::lock_guard<std::mutex> lg(*lock_);
  val_->assign(size_-1, 0, *b);
}

inline void SwLed::evaluate() {
  // Does nothing.
}

inline bool SwLed::there_are_updates() const {
  return false;
}

inline void SwLed::update() { 
  // Does nothing.
}

} // namespace cascade::sw

#endif
