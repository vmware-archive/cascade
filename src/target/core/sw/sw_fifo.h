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

#ifndef CASCADE_SRC_TARGET_CORE_SW_SW_FIFO_H
#define CASCADE_SRC_TARGET_CORE_SW_SW_FIFO_H

#include <cassert>
#include <fstream>
#include <vector>
#include "src/base/bits/bits.h"
#include "src/target/core.h"

namespace cascade {

class SwFifo : public Fifo {
  public:
    // Constructors:
    SwFifo(Interface* interface, size_t depth, size_t byte_size); 
    ~SwFifo() override;

    // Configuration Interface:
    SwFifo& set_file(const std::string& file, size_t count = 1);
    SwFifo& set_clock(VId id);
    SwFifo& set_rreq(VId id);
    SwFifo& set_rdata(VId id);
    SwFifo& set_wreq(VId id);
    SwFifo& set_wdata(VId id);
    SwFifo& set_empty(VId id);
    SwFifo& set_full(VId id);

    State* get_state() override;
    void set_state(const State* s) override;
    Input* get_input() override;
    void set_input(const Input* i) override;

    void read(VId id, const Bits* b) override;
    void evaluate() override;
    bool there_are_updates() const override;
    void update() override;

  private:
    // Source Code:
    VId clock_id_;
    VId rreq_id_;
    VId rdata_id_;
    VId wreq_id_;
    VId wdata_id_;
    VId empty_id_;
    VId full_id_;

    // State:
    std::ifstream ifs_;
    size_t count_;
    std::vector<std::pair<Bits, size_t>> fifo_;
    size_t head_;
    size_t tail_;
    Bits wval_;
    bool push_;
    bool pop_;
    bool posedge_;

    // Local Variables:
    bool clock_;
    bool rreq_;
    bool wreq_;
    Bits wdata_;

    // Scratch Space:
    Bits temp_;

    // Fifo Helpers:
    bool file_empty() const;
    bool fifo_empty() const;
    bool fifo_full() const;
    const Bits* next() const;
    void push(const Bits& b);
    void pop();
};

inline SwFifo::SwFifo(Interface* interface, size_t depth, size_t byte_size) : Fifo(interface) {
  clock_id_ = nullid();
  rreq_id_ = nullid();
  rdata_id_ = nullid();
  wreq_id_ = nullid();
  wdata_id_ = nullid();
  empty_id_ = nullid();
  full_id_ = nullid();

  for (size_t i = 0; i <= depth; ++i) {
    const auto next = i < depth ? i + 1 : 0;
    fifo_.push_back(std::make_pair(Bits(byte_size, 0), next));
  }
  head_ = 0;
  tail_ = 1;
  wval_.resize(byte_size);
  push_ = false;
  pop_ = false;
  posedge_ = false;

  clock_ = false;
  rreq_ = false;
  wreq_ = false;
  wdata_.resize(byte_size);
}

inline SwFifo::~SwFifo() {
  if (ifs_.is_open()) {
    ifs_.close();
  }
}

inline SwFifo& SwFifo::set_file(const std::string& file, size_t count) {
  count_ = count-1;
  ifs_.open(file);
  if (!ifs_.is_open()) {
    return *this;
  }

  while (!fifo_full()) {
    temp_.read(ifs_, 16);
    if (ifs_.eof()) {
      if (count_ > 0) {
        ifs_.clear();
        ifs_.seekg(0);
        continue;
      } else {
        break;
      }
    }
    push(temp_);
  }
  return *this;
}

inline SwFifo& SwFifo::set_clock(VId id) {
  clock_id_ = id;
  return *this;
}

inline SwFifo& SwFifo::set_rreq(VId id) {
  rreq_id_ = id;
  return *this;
}

inline SwFifo& SwFifo::set_rdata(VId id) {
  rdata_id_ = id;
  return *this;
}

inline SwFifo& SwFifo::set_wreq(VId id) {
  wreq_id_ = id;
  return *this;
}

inline SwFifo& SwFifo::set_wdata(VId id) {
  wdata_id_ = id;
  return *this;
}

inline SwFifo& SwFifo::set_empty(VId id) {
  empty_id_ = id;
  return *this;
}

inline SwFifo& SwFifo::set_full(VId id) {
  full_id_ = id;
  return *this;
}

inline State* SwFifo::get_state() {
  VId id = nullid();

  auto s = new State();
  s->insert(++id, Bits(32, ifs_.is_open() ? (uint32_t)ifs_.tellg() : 0));
  s->insert(++id, Bits(32, (uint32_t)count_));
  s->insert(++id, Bits(32, (uint32_t)head_));
  s->insert(++id, Bits(32, (uint32_t)tail_));
  for (const auto& b : fifo_) {
    s->insert(++id, b.first);
  }

  return s;
} 

inline void SwFifo::set_state(const State* s) {
  // TODO: This is a bit lazy. If one variable appears, we assume they all do.
  if (s->begin() == s->end()) {
    return;
  }

  VId id = nullid();
  const auto ptr = s->find(++id)->second.to_int();
  if (ifs_.is_open()) {
    ifs_.seekg(ptr);
  }
  count_ = s->find(++id)->second.to_int();
  head_ = s->find(++id)->second.to_int();
  tail_ = s->find(++id)->second.to_int();
  for (size_t i = 0, ie = fifo_.size(); i < ie; ++i) {
    fifo_[i].first = s->find(++id)->second;
  }
}

inline Input* SwFifo::get_input() {
  auto i = new Input();
  i->insert(clock_id_, Bits(1, clock_));
  i->insert(rreq_id_, Bits(1, rreq_));
  i->insert(wreq_id_, Bits(1, wreq_));
  i->insert(wdata_id_, wdata_);
  return i;
}

inline void SwFifo::set_input(const Input* i) {
  // TODO: This is a bit lazy. If one variable appears, we assume they all do.
  if (i->begin() == i->end()) {
    return;
  }

  clock_ = i->find(clock_id_)->second.to_bool();
  rreq_ = i->find(rreq_id_)->second.to_bool();
  wreq_ = i->find(wreq_id_)->second.to_bool();
  wdata_ = i->find(wdata_id_)->second;
}

inline void SwFifo::read(VId id, const Bits* b) {
  if (id == clock_id_) {
    const auto cold = clock_;
    clock_ = b->to_bool();
    posedge_ = !cold && clock_;
  } else if (id == rreq_id_) {
    rreq_ = b->to_bool();
  } else if (id == wdata_id_) {
    assert(wdata_.size() == b->size());
    wdata_ = *b;
  } else if (id == wreq_id_) {
    wreq_ = b->to_bool();
  } else {
    assert(false);
  }
}

inline void SwFifo::evaluate() {
  if (posedge_) {
    posedge_ = false;
    push_ = wreq_;
    pop_ = rreq_;
    if (wreq_) {
      wval_ = wdata_;
    }
  }
  if (rdata_id_ != nullid()) {
    interface()->write(rdata_id_, next());
  } 
  if (empty_id_ != nullid()) {
    interface()->write(empty_id_, fifo_empty() && file_empty());    
  }
  if (full_id_ != nullid()) {
    interface()->write(full_id_, fifo_full() || !file_empty());    
  }
}

inline bool SwFifo::there_are_updates() const {
  return push_ || pop_;
}

inline void SwFifo::update() { 
  // NOTE: No check here. Popping an empty fifo is undefined. 
  if (pop_) {
    pop_ = false;
    pop(); 
  }
  // NOTE: If we haven't drained the input file, keep pushing. If this
  // condition is true, then we would have reported full. We won't waste time
  // checking for this condition since pushing on a full fifo is undefined.
  while (!file_empty()) {
    temp_.read(ifs_, 16);
    if (!ifs_.eof()) {
      push(temp_);
    } else if (count_ > 0) {
      --count_;
      ifs_.clear();
      ifs_.seekg(0);
      continue;
    } 
    break;
  }
  // Note: No check here. Pushing into a full fifo is undefined.
  if (push_) {
    push_ = false;
    push(wval_);
  }
  // Update continuous state
  evaluate();
}

inline bool SwFifo::file_empty() const {
  return !ifs_.is_open() || ifs_.eof();
}

inline bool SwFifo::fifo_empty() const {
  return fifo_[head_].second == tail_;
}

inline bool SwFifo::fifo_full() const {
  return head_ == tail_;
}

inline const Bits* SwFifo::next() const {
  return &fifo_[head_].first;
}

inline void SwFifo::push(const Bits& b) {
  fifo_[tail_].first.assign(b);
  tail_ = fifo_[tail_].second;
}

inline void SwFifo::pop() {
  head_ = fifo_[head_].second;
}

} // namespace cascade

#endif
