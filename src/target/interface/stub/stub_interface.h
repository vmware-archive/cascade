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

#ifndef CASCADE_SRC_TARGET_INTERFACE_STUB_STUB_INTERFACE_H
#define CASCADE_SRC_TARGET_INTERFACE_STUB_STUB_INTERFACE_H

#include "target/interface.h"

namespace cascade {

class StubInterface : public Interface {
  public:
    StubInterface();
    ~StubInterface() override = default;

    void write(VId id, const Bits* b) override;
    void write(VId id, bool b) override;

    void finish(uint32_t arg) override;
    void restart(const std::string& s) override;
    void retarget(const std::string& s) override;
    void save(const std::string& s) override;

    FId fopen(const std::string& path, uint8_t mode) override;
    int32_t in_avail(FId id) override;
    uint32_t pubseekoff(FId id, int32_t off, uint8_t way, uint8_t which) override;
    uint32_t pubseekpos(FId id, int32_t pos, uint8_t which) override;
    int32_t pubsync(FId id) override;
    int32_t sbumpc(FId id) override;
    int32_t sgetc(FId id) override;
    uint32_t sgetn(FId id, char* c, uint32_t n) override;
    int32_t sputc(FId id, char c) override;
    uint32_t sputn(FId id, const char* c, uint32_t n) override;
};

inline StubInterface::StubInterface() : Interface() { }

inline void StubInterface::write(VId id, const Bits* b) {
  // Does nothing.
  (void) id;
  (void) b;
}

inline void StubInterface::write(VId id, bool b) {
  // Does nothing.
  (void) id;
  (void) b;
}

inline void StubInterface::finish(uint32_t arg) {
  // Does nothing.
  (void) arg;
}

inline void StubInterface::restart(const std::string& s) {
  // Does nothing.
  (void) s;
}

inline void StubInterface::retarget(const std::string& s) {
  // Does nothing.
  (void) s;
}

inline void StubInterface::save(const std::string& s) {
  // Does nothing.
  (void) s;
}

inline FId StubInterface::fopen(const std::string& path, uint8_t mode) {
  // Does nothing
  (void) path;
  (void) mode;
  return 0;
}

inline int32_t StubInterface::in_avail(FId id) {
  // Does nothing
  (void) id;
  return 0;
}

inline uint32_t StubInterface::pubseekoff(FId id, int32_t off, uint8_t way, uint8_t which) {
  // Does nothing
  (void) id;
  (void) off;
  (void) way;
  (void) which;
  return 0;
}

inline uint32_t StubInterface::pubseekpos(FId id, int32_t pos, uint8_t which) {
  // Does nothing
  (void) id;
  (void) pos;
  (void) which;
  return 0;
}

inline int32_t StubInterface::pubsync(FId id) {
  // Does nothing
  (void) id;
  return 0;
}

inline int32_t StubInterface::sbumpc(FId id) {
  // Does nothing.
  (void) id;
  return 0;
}

inline int32_t StubInterface::sgetc(FId id) {
  // Does nothing.
  (void) id;
  return 0;
}

inline uint32_t StubInterface::sgetn(FId id, char* c, uint32_t n) {
  // Does nothing
  (void) id;
  (void) c;
  (void) n;
  return 0;
}

inline int32_t StubInterface::sputc(FId id, char c) {
  // Does nothing.
  (void) id;
  (void) c;
  return 0;
}

inline uint32_t StubInterface::sputn(FId id, const char* c, uint32_t n) {
  // Does nothing
  (void) id;
  (void) c;
  (void) n;
  return 0;
}

} // namespace cascade

#endif
