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

#ifndef CASCADE_SRC_TARGET_INTERFACE_COMPILER_H
#define CASCADE_SRC_TARGET_INTERFACE_COMPILER_H

#include <string>
#include "verilog/ast/ast_fwd.h"

namespace cascade {

class Compiler;
class Interface;

// This class encapsulates target-specific logic for generating target-specific
// instances of module/runtime interfaces. This class is intended to be thread
// safe.  Target-specific implementations of this class must guarantee
// reentrancy.

class InterfaceCompiler {
  public:
    InterfaceCompiler();
    virtual ~InterfaceCompiler() = default;

    // Attaches a reference to the main compiler.
    InterfaceCompiler& set_compiler(Compiler* compiler);

    // Returns a target-specific implementation of a module/runtime interface
    // or nullptr to indicate a failed compilation. In the event of an error,
    // this method must call the error() method to explain what happened. 
    virtual Interface* compile(ModuleDeclaration* md) = 0;

  protected:
    // Logs an error message explaining why the most recent compilation failed.
    // This method is thread safe.
    void error(const std::string& s);

  private:
    // Reference to main compiler
    Compiler* compiler_;
};

} // namespace cascade

#endif
