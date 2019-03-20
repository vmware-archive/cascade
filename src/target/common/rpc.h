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

#ifndef CASCADE_SRC_TARGET_COMMON_RPC_H
#define CASCADE_SRC_TARGET_COMMON_RPC_H

#include <iostream>
#include "src/base/serial/serializable.h"

namespace cascade {

struct Rpc : Serializable {
  enum class Type : uint8_t {
    // Generic Return Codes:
    OKAY = 0,
    FAIL,

    // Compiler API:
    COMPILE,  

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
    DISPLAY,
    ERROR,
    FINISH,
    INFO,
    RESTART,
    RETARGET,
    SAVE,
    WARNING,
    WRITE,
    WRITE_BITS,

    // Teardown Codes:
    ENGINE_TEARDOWN,
    CONNECTION_TEARDOWN
  };
  typedef uint32_t Id;

  Rpc();
  Rpc(Type type, Id id);
  ~Rpc() override = default;

  size_t deserialize(std::istream& is) override;
  size_t serialize(std::ostream& os) const override;

  Type type_;
  Id id_;
};

inline Rpc::Rpc() : Rpc(Type::FAIL, 0) { }

inline Rpc::Rpc(Type type, Id id) : Serializable() {
  type_ = type;
  id_ = id;
}

inline size_t Rpc::deserialize(std::istream& is) {
  is.read(reinterpret_cast<char*>(&type_), sizeof(type_));
  is.read(reinterpret_cast<char*>(&id_), sizeof(id_));
  return sizeof(type_) + sizeof(id_);
}

inline size_t Rpc::serialize(std::ostream& os) const {
  os.write(const_cast<char*>(reinterpret_cast<const char*>(&type_)), sizeof(type_));
  os.write(const_cast<char*>(reinterpret_cast<const char*>(&id_)), sizeof(id_));
  return sizeof(type_) + sizeof(id_);
}

} // namespace cascade

#endif
