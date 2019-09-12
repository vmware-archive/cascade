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

#ifndef CASCADE_SRC_CASCADE_CASCADE_SLAVE_H
#define CASCADE_SRC_CASCADE_CASCADE_SLAVE_H

#include <string>
#include "target/compiler/remote_compiler.h"

namespace cascade {

class CascadeSlave {
  public:
    // Constructors:
    //
    // Only simple construciton is allowed. All other methods of construction
    // are explicitly forbidden.
    CascadeSlave();
    CascadeSlave(const CascadeSlave& rhs) = delete;
    CascadeSlave(CascadeSlave&& rhs) = delete;
    CascadeSlave& operator=(const CascadeSlave& rhs) = delete;
    CascadeSlave& operator=(CascadeSlave&& rhs) = delete;
    ~CascadeSlave();

    // Configuration Methods:
    //
    // These methods should only be called prior to the first invocation of
    // run.  Inoking any of these methods afterwards is undefined.
    CascadeSlave& set_listeners(const std::string& path, size_t port);
    CascadeSlave& set_quartus_server(const std::string& host, size_t port);

    // Start/Stop Methods:
    CascadeSlave& run();
    CascadeSlave& request_stop();
    CascadeSlave& wait_for_stop();
    CascadeSlave& stop_now();

  private:
    RemoteCompiler remote_compiler_;
};

} // namespace cascade

#endif

