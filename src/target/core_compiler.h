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

#include <map>
#include <mutex>
#include <stddef.h>
#include <string>
#include "common/uuid.h"
#include "runtime/ids.h"
#include "target/core.h"
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
    // Returns a target-specific implementation of a module core or nullptr to
    // indicate a failed compilation. This method dispatches control to the
    // protected methods below based on the __std annotation unless either
    // shutdown has been called or version is less than a previously observerd
    // version for this uuid. This method must be thread-safe. Multiple
    // instances may be invoked simulataneously and interleaved with calls to
    // abort().
    Core* compile(const Uuid& uuid, size_t version, ModuleDeclaration* md, Interface* interface);
    // This method must force any running invocation of compile() for uuid to
    // stop running in a 'reasonably short' amount of time. If the compilation
    // would finish, it is safe to return the resulting pointer. Otherwise, the
    // implementation may cause compile to return nullptr.
    virtual void abort(const Uuid& uuid) = 0;
    // Disables the execution of any further compilations and invokes abort on
    // all known uuids.
    void shutdown();

  protected:
    // These methods inherit ownership of md and are responsible for deleting
    // it or passing it off to another owner with the same expectation. In the
    // event of a failed compilation these methods must call the error() method
    // to explain what happened. The default behavior is to delete md, return
    // nullptr, and report that no implementation strategy is available.  The
    // compile_custom() method is invoked in response to a user-defined __std
    // annotation.
    virtual Clock* compile_clock(const Uuid& uuid, ModuleDeclaration* md, Interface* interface);
    virtual Custom* compile_custom(const Uuid& uuid, ModuleDeclaration* md, Interface* interface);
    virtual Gpio* compile_gpio(const Uuid& uuid, ModuleDeclaration* md, Interface* interface);
    virtual Led* compile_led(const Uuid& uuid, ModuleDeclaration* md, Interface* interface);
    virtual Pad* compile_pad(const Uuid& uuid, ModuleDeclaration* md, Interface* interface);
    virtual Reset* compile_reset(const Uuid& uuid, ModuleDeclaration* md, Interface* interface);
    virtual Logic* compile_logic(const Uuid& uuid, ModuleDeclaration* md, Interface* interface);

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
    
    // In-flight compilation index
    std::mutex lock_;
    std::map<Uuid, size_t> compilations_;
    bool shutdown_;
};

} // namespace cascade

#endif
