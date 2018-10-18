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

#ifndef CASCADE_SRC_TARGET_CORE_DE10_DE10_COMPILER_H
#define CASCADE_SRC_TARGET_CORE_DE10_DE10_COMPILER_H

#include <condition_variable>
#include <mutex>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include "src/target/core_compiler.h"
#include "src/target/core/de10/de10_gpio.h"
#include "src/target/core/de10/de10_led.h"
#include "src/target/core/de10/de10_logic.h"
#include "src/target/core/de10/de10_pad.h"
#include "src/target/core/de10/program_boxer.h"

namespace cascade {

class QuartusClient;

class De10Compiler : public CoreCompiler {
  public:
    De10Compiler();
    ~De10Compiler() override;

    De10Compiler& set_host(const std::string& host);
    De10Compiler& set_port(uint32_t port);

    void abort() override;

  private:
    // Memory Mapped State:
    int fd_;
    volatile uint8_t* virtual_base_;

    // Quartus Compiler Location:
    std::string host_;
    uint32_t port_;

    // Compilation Request Ordering:
    std::condition_variable cv_;
    std::mutex lock_;
    ProgramBoxer pbox_;
    size_t curr_seq_;
    size_t next_seq_;
    std::unordered_map<MId, size_t> wait_table_;

    De10Gpio* compile_gpio(Interface* interface, ModuleDeclaration* md) override;
    De10Led* compile_led(Interface* interface, ModuleDeclaration* md) override;
    De10Logic* compile_logic(Interface* interface, ModuleDeclaration* md) override;
    De10Pad* compile_pad(Interface* interface, ModuleDeclaration* md) override;
};

} // namespace cascade

#endif
