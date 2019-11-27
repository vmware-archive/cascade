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

#ifndef CASCADE_SRC_TARGET_CORE_AVMM_VERILATOR_VERILATOR_COMPILER_H
#define CASCADE_SRC_TARGET_CORE_AVMM_VERILATOR_VERILATOR_COMPILER_H

#include <thread>
#include "target/core/avmm/avmm_compiler.h"
#include "target/core/avmm/verilator/verilator_logic.h"
#include "target/core/avmm/avmm_compiler.h"

namespace cascade {

class VerilatorCompiler : public AvmmCompiler<2,12,uint16_t,uint32_t> {
  public:
    VerilatorCompiler();
    ~VerilatorCompiler() override;

    uint32_t read(uint16_t addr) const;
    void write(uint16_t addr, uint32_t val) const;

  private:
    // Avmm Compiler Interface:
    VerilatorLogic* build(Interface* interface, ModuleDeclaration* md, size_t slot) override;
    bool compile(const std::string& text, std::mutex& lock) override;
    void stop_compile() override;

    // Verilator Control Thread:
    std::thread verilator_;

    // Shared Library Handles:
    void* handle_;
    void (*stop_)();
    uint32_t (*read_)(uint16_t);
    void (*write_)(uint16_t, uint32_t);
};

} // namespace cascade

#endif
