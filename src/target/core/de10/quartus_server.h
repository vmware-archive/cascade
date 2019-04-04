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

#ifndef CASCADE_SRC_TARGET_CORE_DE10_QUARTUS_SERVER_H
#define CASCADE_SRC_TARGET_CORE_DE10_QUARTUS_SERVER_H

#include <condition_variable>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "src/base/thread/asynchronous.h"
#include "src/base/thread/thread_pool.h"

namespace cascade {

class sockstream;

class QuartusServer : public Asynchronous {
  public:
    // RPC Interface:
    typedef uint8_t Id;
    enum class Rpc : uint8_t {
      ERROR = 0,
      REQUEST_SLOT,
      UPDATE_SLOT,
      RETURN_SLOT,
      ABORT
    };

    QuartusServer();
    ~QuartusServer() override = default;

    QuartusServer& set_cache_path(const std::string& path);
    QuartusServer& set_quartus_path(const std::string& path);
    QuartusServer& set_port(uint32_t port);
    QuartusServer& set_usb(const std::string& usb);

    bool error() const;

  private:
    enum class State : uint8_t {
      OPEN = 0,
      CURRENT,
      WAITING,
      ABORTED
    };

    std::string cache_path_;
    std::string quartus_path_;
    uint32_t port_;
    std::string usb_;

    ThreadPool pool_;
    std::unordered_map<std::string, std::string> cache_;
    std::vector<std::pair<QuartusServer::State, std::string>> slots_;

    std::mutex lock_;
    std::condition_variable cv_;

    void init_pool();
    void init_cache();
    void init_slots();

    void request_slot(sockstream* sock);
    void update_slot(sockstream* sock);
    void return_slot(sockstream* sock);
    void killall();
    void recompile();
    void abort();

    void run_logic() override;
};

} // namespace cascade

#endif
