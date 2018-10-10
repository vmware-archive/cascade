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

#ifndef CASCADE_SRC_TARGET_CORE_SW_SW_MEMORY_H
#define CASCADE_SRC_TARGET_CORE_SW_SW_MEMORY_H

#include <cassert>
#include <fstream>
#include <vector>
#include "src/base/bits/bits.h"
#include "src/target/core.h"

namespace cascade {

class SwMemory : public Memory {
  public:
    // Constructors:
    SwMemory(Interface* interface, size_t addr_size, size_t byte_size); 
    ~SwMemory() override = default;

    // Configuration Interface:
    SwMemory& set_file(const std::string& file);
    SwMemory& set_clock(VId id);
    SwMemory& set_wen(VId id);
    SwMemory& set_raddr1(VId id);
    SwMemory& set_rdata1(VId id);
    SwMemory& set_raddr2(VId id);
    SwMemory& set_rdata2(VId id);
    SwMemory& set_waddr(VId id);
    SwMemory& set_wdata(VId id);

    State* get_state() override;
    void set_state(const State* s) override;
    Input* get_input() override;
    void set_input(const Input* i) override;

    bool overrides_done_simulation() const override;
    void done_simulation() override;

    void read(VId id, const Bits* b) override;
    void evaluate() override;
    bool there_are_updates() const override;
    void update() override;

  private:
    // Source Code:
    std::string file_;
    VId clock_id_;
    VId wen_id_;
    VId raddr1_id_;
    VId rdata1_id_;
    VId raddr2_id_;
    VId rdata2_id_;
    VId waddr_id_;
    VId wdata_id_;

    // State:
    std::vector<Bits> mem_;
    Bits wval_;
    size_t wdest_;
    bool posedge_;
    bool there_are_updates_;

    // Local Variables:
    bool clock_;
    bool wen_;
    size_t raddr1_;
    size_t raddr2_;
    size_t waddr_;
    Bits wdata_;

    // File I/O Helpers:
    void read_file();
    void write_file();
};

inline SwMemory::SwMemory(Interface* interface, size_t addr_size, size_t byte_size) : Memory(interface) {
  file_ = "";
  clock_id_ = nullid();
  wen_id_ = nullid();
  raddr1_id_ = nullid();
  rdata1_id_ = nullid();
  raddr2_id_ = nullid();
  rdata2_id_ = nullid();
  waddr_id_ = nullid();
  wdata_id_ = nullid();

  mem_.resize(1 << addr_size, Bits(byte_size, 0));
  wval_.resize(byte_size);
  wdest_ = 0;
  posedge_ = false;
  there_are_updates_ = false;

  clock_ = false;
  wen_ = false;
  raddr1_ = 0;
  raddr2_ = 0;
  waddr_ = 0;
  wdata_.resize(byte_size);
}

inline SwMemory& SwMemory::set_file(const std::string& file) {
  file_ = file;
  read_file();
  return *this;
}

inline SwMemory& SwMemory::set_clock(VId id) {
  clock_id_ = id;
  return *this;
}

inline SwMemory& SwMemory::set_wen(VId id) {
  wen_id_ = id;
  return *this;
}

inline SwMemory& SwMemory::set_raddr1(VId id) {
  raddr1_id_ = id;
  return *this;
}

inline SwMemory& SwMemory::set_rdata1(VId id) {
  rdata1_id_ = id;
  return *this;
}

inline SwMemory& SwMemory::set_raddr2(VId id) {
  raddr2_id_ = id;
  return *this;
}

inline SwMemory& SwMemory::set_rdata2(VId id) {
  rdata2_id_ = id;
  return *this;
}

inline SwMemory& SwMemory::set_waddr(VId id) {
  waddr_id_ = id;
  return *this;
}

inline SwMemory& SwMemory::set_wdata(VId id) {
  wdata_id_ = id;
  return *this;
}

inline State* SwMemory::get_state() {
  VId id = nullid();

  auto s = new State();
  for (const auto& m : mem_) {
    s->insert(++id, m);
  }
  return s;
} 

inline void SwMemory::set_state(const State* s) {
  // A bit lazy. If we see one variable, we assume everything is here.
  if (s->begin() == s->end()) {
    return;
  }

  VId id = nullid();
  for (size_t i = 0, ie = mem_.size(); i < ie; ++i) {
    mem_[i] = s->find(++id)->second;
  }
}

inline Input* SwMemory::get_input() {
  auto i = new Input();
  i->insert(clock_id_, Bits(1, clock_));
  i->insert(wen_id_, Bits(1, wen_));
  i->insert(raddr1_id_, Bits(32, (uint32_t)raddr1_));
  i->insert(raddr2_id_, Bits(32, (uint32_t)raddr2_));
  i->insert(waddr_id_, Bits(32, (uint32_t)waddr_));
  i->insert(wdata_id_, wdata_);
  return i;
}

inline void SwMemory::set_input(const Input* i) {
  // A bit lazy. We assume that if one variable appears, they all do.
  if (i->begin() == i->end()) {
    return;
  }
  clock_ = i->find(clock_id_)->second.to_bool();
  wen_ = i->find(wen_id_)->second.to_bool();
  raddr1_ = i->find(raddr1_id_)->second.to_int();
  raddr2_ = i->find(raddr2_id_)->second.to_int();
  waddr_ = i->find(waddr_id_)->second.to_int();
  wdata_ = i->find(wdata_id_)->second;
}

inline bool SwMemory::overrides_done_simulation() const {
  return true;
}

inline void SwMemory::done_simulation() {
  if (file_ != "") {
    write_file();
  }
}

inline void SwMemory::read(VId id, const Bits* b) {
  if (id == clock_id_) {
    const auto cold = clock_;
    clock_ = b->to_bool();
    posedge_ = !cold && clock_;
  } else if (id == wen_id_) {
    wen_ = b->to_bool();
  } else if (id == raddr1_id_) {
    raddr1_ = b->to_int();
  } else if (id == raddr2_id_) {
    raddr2_ = b->to_int();
  } else if (id == waddr_id_) {
    waddr_ = b->to_int();
  } else if (id == wdata_id_) {
    wdata_.assign(*b);
  } else {
    assert(false);
  }
}

inline void SwMemory::evaluate() {
  if (posedge_) {
    posedge_ = false;
    if (wen_) {
      there_are_updates_ = true;
      wval_ = wdata_;
      wdest_ = waddr_;
    }
  }
  if (rdata1_id_ != nullid()) {
    interface()->write(rdata1_id_, &mem_[raddr1_]);
  }
  if (rdata2_id_ != nullid()) {
    interface()->write(rdata2_id_, &mem_[raddr2_]);
  }
}

inline bool SwMemory::there_are_updates() const {
  return there_are_updates_;
}

inline void SwMemory::update() { 
  there_are_updates_ = false;
  mem_[wdest_] = wval_;
  evaluate();
}

inline void SwMemory::read_file() {
  std::ifstream ifs(file_);
  Bits temp;
  for (auto& m : mem_) {
    temp.read(ifs, 16);
    if (ifs.eof()) {
      break;
    }  
    m.assign(m.size()-1, 0, temp);
  }
  ifs.close();
}

inline void SwMemory::write_file() {
  std::ofstream ofs(file_);
  size_t i = 0;
  for (const auto& m : mem_) {
    m.write(ofs, 16);
    ofs << ((++i % 8 == 0) ? "\n" : " ");
  }
  ofs.close();
}

} // namespace cascade

#endif

