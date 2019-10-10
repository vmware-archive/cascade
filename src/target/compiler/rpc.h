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

#ifndef CASCADE_SRC_TARGET_COMPILER_RPC_H
#define CASCADE_SRC_TARGET_COMPILER_RPC_H

#include <iostream>
#include "common/serializable.h"

namespace cascade {

struct Rpc : Serializable {
  enum class Type : uint8_t {
    // Generic Return Codes:
    OKAY = 0,
    FAIL,

    // Compiler API:
    COMPILE,  
    STOP_COMPILE,

    // Core API:
    GET_STATE,
    SET_STATE,
    GET_INPUT,
    SET_INPUT,
    FINALIZE,

    OVERRIDES_DONE_STEP,
    DONE_STEP,
    OVERRIDES_DONE_SIMULATION,
    DONE_SIMULATION,

    READ,
    EVALUATE,
    THERE_ARE_UPDATES,
    UPDATE,
    THERE_WERE_TASKS,

    CONDITIONAL_UPDATE,
    OPEN_LOOP,

    // Interface API:
    WRITE_BITS,
    WRITE_BOOL,

    DEBUG,
    FINISH,
    RESTART,
    RETARGET,
    SAVE,

    FOPEN,
    IN_AVAIL,
    PUBSEEKOFF,
    PUBSEEKPOS,
    PUBSYNC,
    SBUMPC,
    SGETC,
    SGETN,
    SPUTC,
    SPUTN,

    // Proxy Compiler Codes:
    OPEN_CONN_1,
    OPEN_CONN_2,
    CLOSE_CONN,
    STATE_SAFE_BEGIN,
    STATE_SAFE_OKAY,
    STATE_SAFE_FINISH,

    // Proxy Core Codes:
    TEARDOWN_ENGINE
  };

  Rpc();
  Rpc(Type type);
  Rpc(Type type, uint32_t pid, uint32_t eid, uint32_t n);
  ~Rpc() override = default;

  size_t deserialize(std::istream& is) override;
  size_t serialize(std::ostream& os) const override;

  Type type_;
  uint32_t pid_;
  uint32_t eid_;
  uint32_t n_;
};

inline Rpc::Rpc() : Rpc(Type::FAIL, 0, 0, 0) { }

inline Rpc::Rpc(Type type) : Rpc(type, 0, 0, 0) { }

inline Rpc::Rpc(Type type, uint32_t pid, uint32_t eid, uint32_t n) : Serializable() {
  type_ = type;
  pid_ = pid;
  eid_ = eid;
  n_ = n;
}

inline size_t Rpc::deserialize(std::istream& is) {
  is.read(reinterpret_cast<char*>(&type_), sizeof(type_));
  is.read(reinterpret_cast<char*>(&pid_), sizeof(pid_));
  is.read(reinterpret_cast<char*>(&eid_), sizeof(eid_));
  is.read(reinterpret_cast<char*>(&n_), sizeof(n_));
  return sizeof(type_) + sizeof(pid_) + sizeof(eid_) + sizeof(n_);
}

inline size_t Rpc::serialize(std::ostream& os) const {
  os.write(const_cast<char*>(reinterpret_cast<const char*>(&type_)), sizeof(type_));
  os.write(const_cast<char*>(reinterpret_cast<const char*>(&pid_)), sizeof(pid_));
  os.write(const_cast<char*>(reinterpret_cast<const char*>(&eid_)), sizeof(eid_));
  os.write(const_cast<char*>(reinterpret_cast<const char*>(&n_)), sizeof(n_));
  return sizeof(type_) + sizeof(pid_) + sizeof(eid_) + sizeof(n_);
}

} // namespace cascade

#endif
