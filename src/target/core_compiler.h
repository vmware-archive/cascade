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

#ifndef CASCADE_SRC_TARGET_CORE_COMPILER_H
#define CASCADE_SRC_TARGET_CORE_COMPILER_H

#include <stddef.h>
#include <string>
#include "runtime/ids.h"
#include "target/core.h"
#include "target/engine.h"
#include "verilog/ast/ast_fwd.h"

namespace cascade {

class Compiler;

// This class encapsulates target-specific logic for generating target-specific
// instances of module core logic. 

class CoreCompiler {
  public:
    CoreCompiler();
    virtual ~CoreCompiler() = default;

    // Attaches a reference to the main compiler.
    CoreCompiler& set_compiler(Compiler* compiler);

    // Compilation Interface:
    //
    // Returns a target-specific implementation of a Core or nullptr to
    // indicate a failed compilation. This method dispatches control to the
    // protected methods below based on the __std annotation.  Implementations
    // must be thread-safe. 
    Core* compile(Engine::Id id, ModuleDeclaration* md, Interface* interface);
    // Forces any invocation of compile() associated with id to stop running in
    // a *reasonably short* amount of time. If the compilation would finish, it
    // is safe to return the resulting pointer.  Otherwise, an implementation
    // may cause compile() to return nullptr.
    virtual void stop_compile(Engine::Id id) = 0;

  protected:
    // These methods inherit ownership of md and are responsible for deleting
    // it or passing it off to another owner with the same expectation. In the
    // event of a failed compilation these methods must call the error() method
    // to explain what happened. The default behavior is to delete md, return
    // nullptr, and report that no implementation strategy is available.  The
    // compile_custom() method is invoked in response to a user-defined __std
    // annotation.
    virtual Clock* compile_clock(Engine::Id id, ModuleDeclaration* md, Interface* interface);
    virtual Custom* compile_custom(Engine::Id id, ModuleDeclaration* md, Interface* interface);
    virtual Gpio* compile_gpio(Engine::Id id, ModuleDeclaration* md, Interface* interface);
    virtual Led* compile_led(Engine::Id id, ModuleDeclaration* md, Interface* interface);
    virtual Pad* compile_pad(Engine::Id id, ModuleDeclaration* md, Interface* interface);
    virtual Reset* compile_reset(Engine::Id id, ModuleDeclaration* md, Interface* interface);
    virtual Logic* compile_logic(Engine::Id id, ModuleDeclaration* md, Interface* interface);

    // Returns a pointer to this core compiler's enclosing top-level compiler
    Compiler* get_compiler();

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
