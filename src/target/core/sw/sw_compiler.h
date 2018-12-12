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

#ifndef CASCADE_SRC_TARGET_CORE_SW_SW_COMPILER_H
#define CASCADE_SRC_TARGET_CORE_SW_SW_COMPILER_H

#include <mutex>
#include <string>
#include "src/target/core/sw/sw_clock.h"
#include "src/target/core/sw/sw_fifo.h"
#include "src/target/core/sw/sw_led.h"
#include "src/target/core/sw/sw_logic.h"
#include "src/target/core/sw/sw_memory.h"
#include "src/target/core/sw/sw_pad.h"
#include "src/target/core/sw/sw_reset.h"
#include "src/target/core_compiler.h"

namespace cascade {

class SwCompiler : public CoreCompiler {
  public:
    SwCompiler();
    ~SwCompiler() override = default;

    SwCompiler& set_include_dirs(const std::string& s);
    SwCompiler& set_led(Bits* b, std::mutex* l);
    SwCompiler& set_pad(Bits* b, std::mutex* l);
    SwCompiler& set_reset(Bits* b, std::mutex* l);

    void abort() override;

  private:
    SwClock* compile_clock(Interface* interface, ModuleDeclaration* md) override;
    SwFifo* compile_fifo(Interface* interface, ModuleDeclaration* md) override;
    SwLed* compile_led(Interface* interface, ModuleDeclaration* md) override;
    SwLogic* compile_logic(Interface* interface, ModuleDeclaration* md) override;
    SwMemory* compile_memory(Interface* interface, ModuleDeclaration* md) override;
    SwPad* compile_pad(Interface* interface, ModuleDeclaration* md) override;
    SwReset* compile_reset(Interface* interface, ModuleDeclaration* md) override;

    std::string include_dirs_;

    Bits* led_;
    Bits* pad_;
    Bits* reset_;

    std::mutex* led_lock_;
    std::mutex* pad_lock_;
    std::mutex* reset_lock_;
};

} // namespace cascade

#endif
