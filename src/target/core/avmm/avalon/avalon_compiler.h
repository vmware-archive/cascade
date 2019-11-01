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

#ifndef CASCADE_SRC_TARGET_CORE_AVMM_AVALON_AVALON_COMPILER_H
#define CASCADE_SRC_TARGET_CORE_AVMM_AVALON_AVALON_COMPILER_H

#include <fstream>
#include <stdint.h>
#include "cascade/cascade.h"
#include "common/system.h"
#include "target/core/avmm/avalon/avalon_logic.h"
#include "target/core/avmm/avalon/syncbuf.h"
#include "target/core/avmm/avmm_compiler.h"

namespace cascade {

class AvalonCompiler : public AvmmCompiler<uint32_t> {
  public:
    AvalonCompiler();
    ~AvalonCompiler() override;

  private:
    // Avmm Compiler Interface:
    AvalonLogic* build(Interface* interface, ModuleDeclaration* md) override;
    bool compile(const std::string& text, std::mutex& lock) override;
    void stop_compile() override;

    // Slave Cascade:
    Cascade* cascade_;

    // Communication Buffers:
    syncbuf reqs_;
    syncbuf resps_;
};

inline AvalonCompiler::AvalonCompiler() : AvmmCompiler<uint32_t>() {
  cascade_ = nullptr;
}

inline AvalonCompiler::~AvalonCompiler() {
  stop_compile();
}

inline AvalonLogic* AvalonCompiler::build(Interface* interface, ModuleDeclaration* md) {
  return new AvalonLogic(interface, md, reqs_, resps_);
}

inline bool AvalonCompiler::compile(const std::string& text, std::mutex& lock) {
  (void) lock;

  std::ofstream ofs(System::src_root() + "/src/target/core/avmm/avalon/device/program_logic.v");
  ofs << text << std::endl;
  ofs.close();
  
  stop_compile();
  cascade_ = new Cascade();
  cascade_->run();

  const auto ifd = cascade_->open(&reqs_);
  const auto ofd = cascade_->open(&resps_);

  (*cascade_) << "`include \"data/march/minimal.v\"\n";
  (*cascade_) << "integer ifd = " << ifd << ";\n";
  (*cascade_) << "integer ofd = " << ofd << ";\n";
  (*cascade_) << "`include \"src/target/core/avmm/avalon/device/avmm_wrapper.v\"\n";
  (*cascade_) << std::endl;

  return true;
}

inline void AvalonCompiler::stop_compile() {
  if (cascade_ != nullptr) {
    delete cascade_;
    cascade_ = nullptr;
  }
}

} // namespace cascade

#endif
