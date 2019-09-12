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

#ifndef CASCADE_SRC_TARGET_COMPILER_LOCAL_INTERFACE_H
#define CASCADE_SRC_TARGET_COMPILER_LOCAL_INTERFACE_H

#include "runtime/data_plane.h"
#include "runtime/runtime.h"
#include "target/interface.h"

namespace cascade {

class LocalInterface : public Interface {
  public:
    explicit LocalInterface(Runtime* rt);
    ~LocalInterface() override = default;     

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

  private:
    Runtime* rt_;
}; 

inline LocalInterface::LocalInterface(Runtime* rt) : Interface() {
  rt_ = rt;
}

inline void LocalInterface::write(VId id, const Bits* b) {
  rt_->get_data_plane()->write(id, b);
}

inline void LocalInterface::write(VId id, bool b) {
  rt_->get_data_plane()->write(id, b);
}

inline void LocalInterface::finish(uint32_t arg) {
  rt_->finish(arg);
}

inline void LocalInterface::restart(const std::string& s) {
  rt_->restart(s);
}

inline void LocalInterface::retarget(const std::string& s) {
  rt_->retarget(s);
}

inline void LocalInterface::save(const std::string& s) {
  rt_->save(s);
}

inline FId LocalInterface::fopen(const std::string& path, uint8_t mode) {
  return rt_->fopen(path, mode);
}

inline int32_t LocalInterface::in_avail(FId id) {
  return rt_->in_avail(id);
}

inline uint32_t LocalInterface::pubseekoff(FId id, int32_t off, uint8_t way, uint8_t which) {
  return rt_->pubseekoff(id, off, way, which);
}

inline uint32_t LocalInterface::pubseekpos(FId id, int32_t pos, uint8_t which) {
  return rt_->pubseekpos(id, pos, which);
}

inline int32_t LocalInterface::pubsync(FId id) {
  return rt_->pubsync(id);
}

inline int32_t LocalInterface::sbumpc(FId id) {
  return rt_->sbumpc(id);
}

inline int32_t LocalInterface::sgetc(FId id) {
  return rt_->sgetc(id);
}

inline uint32_t LocalInterface::sgetn(FId id, char* c, uint32_t n) {
  return rt_->sgetn(id, c, n);
}

inline int32_t LocalInterface::sputc(FId id, char c) {
  return rt_->sputc(id, c);
}

inline uint32_t LocalInterface::sputn(FId id, const char* c, uint32_t n) {
  return rt_->sputn(id, c, n);
}

} // namespace cascade

#endif
