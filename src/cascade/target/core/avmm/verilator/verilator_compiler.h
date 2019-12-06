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

#include <cstdlib>
#include <dlfcn.h>
#include <fstream>
#include <thread>
#include <type_traits>
#include "common/system.h"
#include "target/core/avmm/avmm_compiler.h"
#include "target/core/avmm/verilator/verilator_logic.h"
#include "target/core/avmm/avmm_compiler.h"

namespace cascade::avmm {

template <size_t M, size_t V, typename A, typename T>
class VerilatorCompiler : public AvmmCompiler<M,V,A,T> {
  public:
    VerilatorCompiler();
    ~VerilatorCompiler() override;

  private:
    // Avmm Compiler Interface:
    VerilatorLogic<V,A,T>* build(Interface* interface, ModuleDeclaration* md, size_t slot) override;
    bool compile(const std::string& text, std::mutex& lock) override;
    void stop_compile() override;

    // Verilator Control Thread:
    std::thread verilator_;

    // Shared Library Handles:
    void* handle_;
    void (*stop_)();

    // Logic Core Handle:
    VerilatorLogic<V,A,T>* logic_;
};

using Verilator32Compiler = VerilatorCompiler<2,12,uint16_t,uint32_t>;
using Verilator64Compiler = VerilatorCompiler<8,20,uint32_t,uint64_t>;

template <size_t M, size_t V, typename A, typename T>
inline VerilatorCompiler<M,V,A,T>::VerilatorCompiler() : AvmmCompiler<M,V,A,T>() {
  handle_ = nullptr;
}

template <size_t M, size_t V, typename A, typename T>
inline VerilatorCompiler<M,V,A,T>::~VerilatorCompiler() {
  if (handle_ != nullptr) {
    stop_();
    verilator_.join();
    dlclose(handle_);
  }
}

template <size_t M, size_t V, typename A, typename T>
inline VerilatorLogic<V,A,T>* VerilatorCompiler<M,V,A,T>::build(Interface* interface, ModuleDeclaration* md, size_t slot) {
  logic_ = new VerilatorLogic<V,A,T>(interface, md, slot);
  return logic_;
}

template <size_t M, size_t V, typename A, typename T>
inline bool VerilatorCompiler<M,V,A,T>::compile(const std::string& text, std::mutex& lock) {
  stop_compile();

  System::execute("mkdir -p /tmp/verilator/");
  char path[] = "/tmp/verilator/program_logic_XXXXXX.v";
  const auto fd = mkstemps(path, 2);
  const auto dir = std::string(path).substr(0,35);
  close(fd);

  System::execute("mkdir -p " + dir);
  std::ofstream ofs(path);
  ofs << text << std::endl;
  ofs.close();

  pid_t pid = 0;
  if constexpr (std::is_same<T, uint32_t>::value) {
    pid = System::no_block_begin_execute("cd " + System::src_root() + "/share/cascade/verilator/ && ./build_verilator_32.sh " + dir, false);
  } else if constexpr (std::is_same<T, uint64_t>::value) {
    pid = System::no_block_begin_execute("cd " + System::src_root() + "/share/cascade/verilator/ && ./build_verilator_64.sh " + dir, false);
  } 

  lock.unlock();
  const auto res = System::no_block_wait_finish(pid);
  lock.lock();

  if (res != 0) {
    return false;
  }
    
  AvmmCompiler<M,V,A,T>::get_compiler()->schedule_state_safe_interrupt([this, dir]{
    if (handle_ != nullptr) {
      stop_();
      verilator_.join();
      dlclose(handle_);
    }
    
    handle_ = dlopen((dir + "/libverilator.so").c_str(), RTLD_LAZY);
    stop_ = (void (*)()) dlsym(handle_, "verilator_stop");
    
    auto read = (T (*)(A)) dlsym(handle_, "verilator_read");
    auto write = (void (*)(A, T)) dlsym(handle_, "verilator_write");
    logic_->set_io(read, write);
    
    auto init = (void (*)()) dlsym(handle_, "verilator_init");
    init();
    auto start = (void (*)()) dlsym(handle_, "verilator_start");
    verilator_ = std::thread(start);
  });

  return true;
}

template <size_t M, size_t V, typename A, typename T>
inline void VerilatorCompiler<M,V,A,T>::stop_compile() {
  if constexpr (std::is_same<T, uint32_t>::value) {
    System::execute(R"(pkill -9 -P `ps -ax | grep build_verilator_32.sh | awk '{print $1}' | head -n1`)");
  } else if constexpr (std::is_same<T, uint64_t>::value) {
    System::execute(R"(pkill -9 -P `ps -ax | grep build_verilator_64.sh | awk '{print $1}' | head -n1`)");
  } 
}

} // namespace cascade::avmm

#endif
