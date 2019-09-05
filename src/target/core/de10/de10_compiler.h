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

#ifndef CASCADE_SRC_TARGET_CORE_DE10_DE10_COMPILER_H
#define CASCADE_SRC_TARGET_CORE_DE10_DE10_COMPILER_H

#include <condition_variable>
#include <mutex>
#include <stdint.h>
#include <string>
#include <vector>
#include "target/core_compiler.h"
#include "target/core/de10/de10_gpio.h"
#include "target/core/de10/de10_led.h"
#include "target/core/de10/de10_logic.h"
#include "target/core/de10/de10_pad.h"

namespace cascade {

class sockstream;

class De10Compiler : public CoreCompiler {
  public:
    De10Compiler();
    ~De10Compiler() override;

    De10Compiler& set_host(const std::string& host);
    De10Compiler& set_port(uint32_t port);

    void release(size_t slot);
    void stop_compile(Engine::Id id) override;
    void stop_async() override;

  private:
    // Compilation States:
    enum class State : uint8_t {
      FREE = 0,
      WAITING,
      CURRENT
    };
    // Slot Information:
    struct Slot {
      Engine::Id id;
      State state;
      std::string text;
    };

    // Configuration State:
    std::string host_;
    uint32_t port_;

    // Memory Mapped State:
    int fd_;
    volatile uint8_t* virtual_base_;

    // Program Management:
    std::mutex lock_;
    std::condition_variable cv_;
    size_t sequence_;
    std::vector<Slot> slots_;

    // Compiler Interface:
    De10Gpio* compile_gpio(Engine::Id id, ModuleDeclaration* md, Interface* interface) override;
    De10Led* compile_led(Engine::Id id, ModuleDeclaration* md, Interface* interface) override;
    De10Logic* compile_logic(Engine::Id id, ModuleDeclaration* md, Interface* interface) override;
    De10Pad* compile_pad(Engine::Id id, ModuleDeclaration* md, Interface* interface) override;

    // Compilation Helpers:
    void kill_all(sockstream* sock);
    void compile(sockstream* sock);
    bool block_on_compile(sockstream* sock);
    void reprogram(sockstream* sock);
};

} // namespace cascade

#endif
