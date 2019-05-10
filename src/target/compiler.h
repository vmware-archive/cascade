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

#ifndef CASCADE_SRC_TARGET_COMPILER_H
#define CASCADE_SRC_TARGET_COMPILER_H

#include <mutex>
#include <string>
#include "base/thread/thread_pool.h"
#include "verilog/ast/ast_fwd.h"
#include "verilog/ast/visitors/editor.h"
#include "verilog/ast/visitors/visitor.h"

namespace cascade {

class CoreCompiler;
class De10Compiler;
class Engine;
class InterfaceCompiler;
class LocalCompiler;
class ProxyCompiler;
class RemoteCompiler;
class Runtime;
class SwCompiler;

class Compiler {
  public:
    // Constructors:
    Compiler();
    ~Compiler();

    // Non-Tread-Safe Core Compiler Configuration:
    // These methods must only be called before the first invocation of
    // compile() and they cannot be called more than once. Once you configure a
    // compiler, you're stuck with it.
    Compiler& set_de10_compiler(De10Compiler* c);
    Compiler& set_proxy_compiler(ProxyCompiler* c);
    Compiler& set_sw_compiler(SwCompiler* c);

    // Non-Thread-Safe Interface Compiler Configuration:
    // These methods must only be called before the first invocation of compile()
    // and they cannot be called more than once. Once you configure a compiler,
    // you're stuck with it.
    Compiler& set_local_compiler(LocalCompiler* c);
    Compiler& set_remote_compiler(RemoteCompiler* c);

    // Compilation Interface:
    // 
    // Compiles a module declaration into a new engine. Returns nullptr if
    // compilation was aborted (either due to premature termination or error).
    // In the case of error, the thread safe interface will indicate what
    // happened.
    Engine* compile(ModuleDeclaration* md);
    // Compiles a module declaration into a new engine, transfers the runtime
    // state of a preexisting engine into the new engine, and then swaps the
    // identities of the two engines (effectively updating the compilation
    // state of the original engine). This method may also use the runtime's
    // interrupt interface to schedule a slower compilation and an asynchronous
    // jit update to the engine at a later time. In the event of an error, the
    // thread safe interface will indicate what happened. In the event that md
    // has been mangled beyond recognition, id can be used to provide a legible
    // identifier for the sake of logging.
    void compile_and_replace(Runtime* rt, Engine* e, size_t& version, ModuleDeclaration* md, const Identifier* id);

    // Thread-Safe Error Interface:
    void error(const std::string& s);
    bool error();
    std::string what();

  private:
    // Core Compilers:
    De10Compiler* de10_compiler_;
    ProxyCompiler* proxy_compiler_;
    SwCompiler* sw_compiler_;

    // Interface Compilers:
    LocalCompiler* local_compiler_;
    RemoteCompiler* remote_compiler_;

    // JIT State:
    ThreadPool pool_;
    size_t seq_compile_;
    size_t seq_build_;

    // Error State:
    std::mutex lock_;
    std::string what_;

    // Helper Class: Checks whether a module is a stub, that is, whether it
    // contains system tasks or initial constructs, or it has non-empty input
    // or output sets.
    class StubCheck : public Visitor {
      public:
        ~StubCheck() override = default;
        bool check(const ModuleDeclaration* md);
      private:
        bool stub_;
        void visit(const InitialConstruct* ic) override;
        void visit(const DisplayStatement* ds) override;
        void visit(const FinishStatement* fs) override;
        void visit(const WriteStatement* ws) override;
    };    
};

} // namespace cascade

#endif
