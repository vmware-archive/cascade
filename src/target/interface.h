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
#include "src/base/bits/bits.h"
#include "src/runtime/ids.h"

namespace cascade {

// This module encapsulates the mechanism by which a core communicates values
// and system task executions back to the runtime.

class Interface {
  public:
    virtual ~Interface() = default;

    // These methods must perform whatever target-specific logic is necessary to
    // cause the corresponding system task calls to be invoked in the runtime.
    virtual void display(const std::string& s) = 0;
    virtual void error(const std::string& s) = 0;
    virtual void finish(int arg) = 0;
    virtual void info(const std::string& s) = 0;
    virtual void restart(const std::string& s) = 0;
    virtual void retarget(const std::string& s) = 0;
    virtual void save(const std::string& s) = 0;
    virtual void warning(const std::string& s) = 0;
    virtual void write(const std::string& s) = 0;

    // These methods must perform whatever target-specific logic is necessary to
    // communicate changes in the value of logic elements back to the runtime.
    virtual void write(VId id, const Bits* b) = 0;
    // Target-specific implementations may override this method if there is a
    // performance-specific advantage to doing so. This method may be invoked
    // whenever a write of a single-bit variable is required.
    virtual void write(VId id, bool b);

    // These methods must perform whatever target-specific logic is necessary
    // to cause the corresponding API calls to be invoked by the runtime.
    virtual SId fopen(const std::string& path) = 0;
    virtual void close(SId id) = 0;
    virtual void seekoff(SId id, int n, bool r) = 0;
    virtual size_t sgetn(SId id, char* c, size_t n) = 0;
    virtual void sputn(SId id, const char* c, size_t n) = 0;
    virtual int in_avail(SId id) = 0;
};

inline void Interface::write(VId id, bool b) {
  Bits temp;
  temp.set(0, b ? 1 : 0);
  write(id, &temp);
}

} // namespace cascade

#endif
