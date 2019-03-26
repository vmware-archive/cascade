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

#include "src/target/interface.h"

namespace cascade {

class StubInterface : public Interface {
  public:
    StubInterface();
    ~StubInterface() override = default;

    void display(const std::string& s) override;
    void error(const std::string& s) override;
    void finish(uint32_t arg) override;
    void info(const std::string& s) override;
    void restart(const std::string& s) override;
    void retarget(const std::string& s) override;
    void save(const std::string& s) override;
    void warning(const std::string& s) override;
    void write(const std::string& s) override;

    void write(VId id, const Bits* b) override;

    SId fopen(const std::string& path) override;
    void close(SId id) override;
    int32_t in_avail(SId id) override;
    uint32_t pubseekoff(SId id, int32_t n, bool r) override;
    uint32_t pubseekpos(SId id, int32_t n, bool r) override;
    int32_t pubsync(SId id) override;
    int32_t sbumpc(SId id) override;
    int32_t sgetc(SId id) override;
    uint32_t sgetn(SId id, char* c, uint32_t n) override;
    int32_t sputc(SId id, char c) override;
    uint32_t sputn(SId id, const char* c, uint32_t n) override;
};

inline StubInterface::StubInterface() : Interface() { }

inline void StubInterface::display(const std::string& s) {
  // Does nothing.
  (void) s;
}

inline void StubInterface::error(const std::string& s) {
  // Does nothing.
  (void) s;
}

inline void StubInterface::finish(uint32_t arg) {
  // Does nothing.
  (void) arg;
}

inline void StubInterface::info(const std::string& s) {
  // Does nothing.
  (void) s;
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

inline void StubInterface::warning(const std::string& s) {
  // Does nothing.
  (void) s;
}

inline void StubInterface::write(const std::string& s) {
  // Does nothing.
  (void) s;
}

inline void StubInterface::write(VId id, const Bits* b) {
  // Does nothing.
  (void) id;
  (void) b;
}

inline SId StubInterface::fopen(const std::string& path) {
  // Does nothing
  (void) path;
  return 0;
}

inline void StubInterface::close(SId id) {
  // Does nothing
  (void) id;
}

inline int32_t StubInterface::in_avail(SId id) {
  // Does nothing
  (void) id;
  return 0;
}

inline uint32_t StubInterface::pubseekoff(SId id, int32_t n, bool r) {
  // Does nothing
  (void) id;
  (void) n;
  (void) r;
  return 0;
}

inline uint32_t StubInterface::pubseekpos(SId id, int32_t n, bool r) {
  // Does nothing
  (void) id;
  (void) n;
  (void) r;
  return 0;
}

inline int32_t StubInterface::pubsync(SId id) {
  // Does nothing
  (void) id;
  return 0;
}

inline int32_t StubInterface::sbumpc(SId id) {
  // Does nothing.
  (void) id;
  return 0;
}

inline int32_t StubInterface::sgetc(SId id) {
  // Does nothing.
  (void) id;
  return 0;
}

inline uint32_t StubInterface::sgetn(SId id, char* c, uint32_t n) {
  // Does nothing
  (void) id;
  (void) c;
  (void) n;
  return 0;
}

inline int32_t StubInterface::sputc(SId id, char c) {
  // Does nothing.
  (void) id;
  (void) c;
  return 0;
}

inline uint32_t StubInterface::sputn(SId id, const char* c, uint32_t n) {
  // Does nothing
  (void) id;
  (void) c;
  (void) n;
  return 0;
}

} // namespace cascade

#endif
