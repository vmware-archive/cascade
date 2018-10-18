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

#ifndef CASCADE_SRC_TARGET_CORE_DE10_QUARTUS_SERVER_H
#define CASCADE_SRC_TARGET_CORE_DE10_QUARTUS_SERVER_H

#include <string>
#include "src/base/thread/asynchronous.h"
#include "src/base/stream/bufstream.h"

namespace cascade {

class Socket;

class QuartusServer : public Asynchronous {
  public:
    QuartusServer();
    ~QuartusServer() override = default;

    QuartusServer& path(const std::string& path);
    QuartusServer& usb(const std::string& usb);
    QuartusServer& port(uint32_t port);

    bool check() const;

  private:
    class Worker : public Asynchronous {
      public:
        Worker(QuartusServer* qs);
        ~Worker() = default;
        void run_logic() override;
      private:
        QuartusServer* qs_;
    };

    std::string path_;
    std::string usb_;
    uint32_t port_;

    Socket* sock_;
    bufstream buf_;
    Worker worker_;

    void run_logic() override;

};

} // namespace cascade

#endif
