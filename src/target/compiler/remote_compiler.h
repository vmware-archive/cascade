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

#ifndef CASCADE_SRC_TARGET_COMPILER_REMOTE_COMPILER_H
#define CASCADE_SRC_TARGET_COMPILER_REMOTE_COMPILER_H

#include <string>
#include "common/thread.h"
#include "target/compiler.h"
#include "target/compiler/rpc.h"

namespace cascade {

class sockstream;

class RemoteCompiler : public Compiler, public Thread {
  public:
    RemoteCompiler();
    ~RemoteCompiler() override = default;

    RemoteCompiler& set_path(const std::string& p);
    RemoteCompiler& set_port(uint32_t p);

  private:
    // Configuration Options:
    std::string path_;
    uint32_t port_;

    // Interface Compilation State:
    sockstream* sock_;
    Rpc::Id id_;

    // Compiler Interface:
    void schedule_state_safe_interrupt(Runtime::Interrupt int_) override;
    void schedule_asynchronous(Runtime::Asynchronous async) override;
    Interface* get_interface(const std::string& loc) override;

    // Thread Interface:
    void run_logic() override;

    // Compiler Interface:
    Engine* compile(sockstream* sock);
    void abort(sockstream* sock);
    
    // Core Interface:
    void get_state(sockstream* sock, Engine* e);
    void set_state(sockstream* sock, Engine* e);
    void get_input(sockstream* sock, Engine* e);
    void set_input(sockstream* sock, Engine* e);
    void finalize(sockstream* sock, Rpc::Id id, Engine* e);

    void overrides_done_step(sockstream* sock, Engine* e);
    void done_step(sockstream* sock, Engine* e);
    void overrides_done_simulation(sockstream* sock, Engine* e);
    void done_simulation(sockstream* sock, Engine* e);

    void read(sockstream* sock, Engine* e);
    void evaluate(sockstream* sock, Rpc::Id id, Engine* e);
    void there_are_updates(sockstream* sock, Engine* e);
    void update(sockstream* sock, Rpc::Id id, Engine* e);
    void there_were_tasks(sockstream* sock, Engine* e);

    void conditional_update(sockstream* sock, Rpc::Id id, Engine* e);
    void open_loop(sockstream* sock, Rpc::Id id, Engine* e);
};

} // namespace cascade

#endif

