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

#ifndef CASCADE_SRC_TARGET_INTERFACE_H
#define CASCADE_SRC_TARGET_INTERFACE_H

#include <string>
#include "common/bits.h"
#include "runtime/ids.h"

namespace cascade {

class InterfaceCompiler;

// This module encapsulates the mechanism by which a core communicates values
// and system task executions back to the runtime.

class Interface {
  public:
    virtual ~Interface() = default;

    // These methods must perform whatever target-specific logic is necessary to
    // invoke the corresponding data-plane methods on the runtime.
    virtual void write(VId id, const Bits* b) = 0;
    virtual void write(VId id, bool b) = 0;

    // These methods must perform whatever target-specific logic is necessary to
    // invoke the corresponding system task calls on the runtime.
    virtual void finish(uint32_t arg) = 0;
    virtual void restart(const std::string& s) = 0;
    virtual void retarget(const std::string& s) = 0;
    virtual void save(const std::string& s) = 0;

    // These methods must perform whatever target-specific logic is necessary
    // to invoke the corresponding stream calls on the runtime.
    virtual FId fopen(const std::string& path, uint8_t mode) = 0;
    virtual int32_t in_avail(FId id) = 0;
    virtual uint32_t pubseekoff(FId id, int32_t off, uint8_t way, uint8_t which) = 0;
    virtual uint32_t pubseekpos(FId id, int32_t pos, uint8_t which) = 0;
    virtual int32_t pubsync(FId id) = 0;
    virtual int32_t sbumpc(FId id) = 0;
    virtual int32_t sgetc(FId id) = 0;
    virtual uint32_t sgetn(FId id, char* c, uint32_t n) = 0;
    virtual int32_t sputc(FId id, char c) = 0;
    virtual uint32_t sputn(FId id, const char* c, uint32_t n) = 0;

    // Target specific implementations may override this method to perform
    // last-minute cleanup in the compiler that created this interface.
    virtual void cleanup(InterfaceCompiler* ic);
};

inline void Interface::cleanup(InterfaceCompiler* ic) {
  // Does nothing.
  (void) ic;
}

} // namespace cascade

#endif
