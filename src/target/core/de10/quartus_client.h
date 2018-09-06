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

#ifndef CASCADE_SRC_TARGET_CORE_DE10_QUARTUS_CLIENT_H
#define CASCADE_SRC_TARGET_CORE_DE10_QUARTUS_CLIENT_H

#include <condition_variable>
#include <mutex>
#include <string>
#include <unordered_map>
#include "src/base/thread/asynchronous.h"
#include "src/runtime/ids.h"
#include "src/verilog/analyze/indices.h"

namespace cascade {

class QuartusClient : public Asynchronous {
  public:
    QuartusClient();
    ~QuartusClient() override = default;

    QuartusClient& set_batch_window(size_t bw);
    QuartusClient& set_host(const std::string& host);
    QuartusClient& set_port(uint32_t port);

    bool refresh(MId id, const std::string& src);

  private:
    size_t batch_window_;
    std::string host_;
    uint32_t port_;

    std::mutex lock_;
    std::condition_variable cv_;

    bool refresh_requested_;
    bool error_;
    std::unordered_map<MId, std::string> src_;

    void sync();
    void run_logic() override;
};

} // namespace cascade

#endif
