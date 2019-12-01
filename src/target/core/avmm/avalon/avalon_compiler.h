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
#include <type_traits>
#include "cascade/cascade.h"
#include "common/system.h"
#include "target/core/avmm/avmm_compiler.h"
#include "target/core/avmm/avalon/avalon_logic.h"
#include "target/core/avmm/avalon/syncbuf.h"
#include "target/core/avmm/avmm_compiler.h"

namespace cascade {

template <size_t M, size_t V, typename A, typename T>
class AvalonCompiler : public AvmmCompiler<M,V,A,T> {
  public:
    AvalonCompiler();
    ~AvalonCompiler() override;

  private:
    // Avmm Compiler Interface:
    AvalonLogic<V,A,T>* build(Interface* interface, ModuleDeclaration* md, size_t slot) override;
    bool compile(const std::string& text, std::mutex& lock) override;
    void stop_compile() override;

    // Slave Cascade:
    Cascade* cascade_;

    // Communication Buffers:
    syncbuf reqs_;
    syncbuf resps_;
};

using Avalon32Compiler = AvalonCompiler<2,12,uint16_t,uint32_t>;
using Avalon64Compiler = AvalonCompiler<8,20,uint32_t,uint64_t>;

template <size_t M, size_t V, typename A, typename T>
inline AvalonCompiler<M,V,A,T>::AvalonCompiler() : AvmmCompiler<M,V,A,T>() {
  cascade_ = nullptr;
}

template <size_t M, size_t V, typename A, typename T>
inline AvalonCompiler<M,V,A,T>::~AvalonCompiler() {
  if (cascade_ != nullptr) {
    delete cascade_;
  }
}

template <size_t M, size_t V, typename A, typename T>
inline AvalonLogic<V,A,T>* AvalonCompiler<M,V,A,T>::build(Interface* interface, ModuleDeclaration* md, size_t slot) {
  return new AvalonLogic<V,A,T>(interface, md, slot, &reqs_, &resps_);
}

template <size_t M, size_t V, typename A, typename T>
inline bool AvalonCompiler<M,V,A,T>::compile(const std::string& text, std::mutex& lock) {
  (void) lock;
  AvalonCompiler<M,V,A,T>::get_compiler()->schedule_state_safe_interrupt([this, &text]{
    std::ofstream ofs(System::src_root() + "/var/avalon/program_logic.v");
    ofs << text << std::endl;
    ofs.close();
  
    if (cascade_ != nullptr) {
      delete cascade_;
    }
    cascade_ = new Cascade();
    cascade_->set_stdout(std::cout.rdbuf());
    cascade_->set_stderr(std::cout.rdbuf());

    const auto ifd = cascade_->open(&reqs_);
    const auto ofd = cascade_->open(&resps_);

    cascade_->run();
    *cascade_ << "`include \"share/march/regression/minimal.v\"\n";
    *cascade_ << "integer ifd = " << ifd << ";\n";
    *cascade_ << "integer ofd = " << ofd << ";\n";
    if constexpr (std::is_same<T, uint32_t>::value) {
      *cascade_ << "`include \"var/avalon/avalon32_wrapper.v\"\n";
    } else if constexpr (std::is_same<T, uint64_t>::value) {
      *cascade_ << "`include \"var/avalon/avalon64_wrapper.v\"\n";
    }
    *cascade_ << std::endl;
  });

  return true;
}

template <size_t M, size_t V, typename A, typename T>
inline void AvalonCompiler<M,V,A,T>::stop_compile() {
  // Does nothing. 
}

} // namespace cascade

#endif
