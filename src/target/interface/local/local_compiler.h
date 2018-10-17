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

#ifndef CASCADE_SRC_TARGET_INTERFACE_LOCAL_LOCAL_COMPILER_H
#define CASCADE_SRC_TARGET_INTERFACE_LOCAL_LOCAL_COMPILER_H

#include "src/target/interface_compiler.h"
#include "src/target/interface/local/local_interface.h"

namespace cascade {

class Runtime;

class LocalCompiler : public InterfaceCompiler {
  public:
    LocalCompiler();
    ~LocalCompiler() override = default;

    LocalCompiler& set_runtime(Runtime* rt);

    LocalInterface* compile(ModuleDeclaration* md) override;
    void teardown(Interface* interface) override;

  private:
    Runtime* rt_;
};

inline LocalCompiler::LocalCompiler() : InterfaceCompiler() {
  rt_ = nullptr;
}

inline LocalCompiler& LocalCompiler::set_runtime(Runtime* rt) {
  rt_ = rt;
  return *this;
}

inline LocalInterface* LocalCompiler::compile(ModuleDeclaration* md) {
  (void) md;
  if (rt_ == nullptr) {
    error("Unable to compile a local interface without a reference to the runtime");
    return nullptr;
  }
  return new LocalInterface(rt_);
}

inline void LocalCompiler::teardown(Interface* interface) {
  // Does nothing.
  (void) interface;
}

} // namespace cascade

#endif
