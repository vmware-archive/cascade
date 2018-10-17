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

#ifndef CASCADE_SRC_TARGET_CORE_COMPILER_H
#define CASCADE_SRC_TARGET_CORE_COMPILER_H

#include <stddef.h>
#include <string>
#include "src/runtime/ids.h"
#include "src/target/core.h"
#include "src/verilog/ast/ast_fwd.h"

namespace cascade {

class Compiler;

// This class encapsulates target-specific logic for generating target-specific
// instances of module core logic. This class is intended to be thread safe.
// Target-specific implementations of this class must guarantee reentrancy.

class CoreCompiler {
  public:
    CoreCompiler();
    virtual ~CoreCompiler() = default;

    // Attaches a reference to the main compiler.
    CoreCompiler& set_compiler(Compiler* compiler);

    // Returns a target-specific implementation of a module core or nullptr to
    // indicate an aborted compilation. This method dispatches control to the
    // protected methods below based on the value of the __std annotation. 
    Core* compile(Interface* interface, ModuleDeclaration* md);
    // This method must teardown any exogenous state associated with an engine
    // core.  Access to an engine core once it has been torn down is undefined.
    virtual void teardown(Core* core) = 0;

  protected:
    // These methods may be overriden to provide a target-specific
    // implementation of each of the standard library module types or nullptr
    // to indicate an aborted compilation. In the event of an error, these
    // methods must call the error() method to explain what happened. The
    // default behavior is to delete md, return nullptr, and report that no
    // implementation strategy is available. 
    virtual Clock* compile_clock(Interface* interface, ModuleDeclaration* md);
    virtual Fifo* compile_fifo(Interface* interface, ModuleDeclaration* md);
    virtual Gpio* compile_gpio(Interface* interface, ModuleDeclaration* md);
    virtual Led* compile_led(Interface* interface, ModuleDeclaration* md);
    virtual Memory* compile_memory(Interface* interface, ModuleDeclaration* md);
    virtual Pad* compile_pad(Interface* interface, ModuleDeclaration* md);
    virtual Reset* compile_reset(Interface* interface, ModuleDeclaration* md);
    virtual Logic* compile_logic(Interface* interface, ModuleDeclaration* md);

    // Logs an error message explaining why the most recent compilation failed.
    // This method is thread safe.
    void error(const std::string& s);

    // Returns the canonical name for a variable identifier.
    MId to_mid(const Identifier* id) const;
    // Returns the canonical name for a module identifier.
    VId to_vid(const Identifier* id) const;
    // Checks whether a module has at most one input of arity up to is and at
    // most one output of arity os.
    bool check_io(const ModuleDeclaration* md, size_t is, size_t os) const;

  private:
    // Reference to main compiler
    Compiler* compiler_;
};

} // namespace cascade

#endif
