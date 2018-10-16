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

#ifndef CASCADE_SRC_TARGET_COMPILER_H
#define CASCADE_SRC_TARGET_COMPILER_H

#include "src/base/thread/thread_pool.h"
#include "src/verilog/ast/ast_fwd.h"
#include "src/verilog/ast/visitors/editor.h"
#include "src/verilog/ast/visitors/visitor.h"

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

    // Configuration Interface:
    Compiler& set_runtime(Runtime* rt);
    Compiler& set_num_jit_threads(size_t n);

    // Core Compiler Configuration:
    Compiler& set_de10_compiler(De10Compiler* c);
    Compiler& set_proxy_compiler(ProxyCompiler* c);
    Compiler& set_sw_compiler(SwCompiler* c);

    // Interface Compiler Configuration:
    Compiler& set_local_compiler(LocalCompiler* c);
    Compiler& set_remote_compiler(RemoteCompiler* c);

    // Compilation Interface:
    Engine* compile(ModuleDeclaration* md);
    Engine* compile_and_replace(Engine* e, ModuleDeclaration* md);

  private:
    // Core Compilers:
    De10Compiler* de10_compiler_;
    ProxyCompiler* proxy_compiler_;
    SwCompiler* sw_compiler_;

    // Interface Compilers:
    LocalCompiler* local_compiler_;
    RemoteCompiler* remote_compiler_;

    // JIT State:
    Runtime* rt_;
    ThreadPool pool_;

    // Checks whether a module is a stub, that is, whether it contains system
    // tasks or initial constructs, or it has non-empty input or output sets.
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

    // Annotates initial statements with ignore comments
    class Masker : public Editor {
      public:
        ~Masker() override = default;
        void mask(ModuleDeclaration* md);
      private:
        void edit(InitialConstruct* ic) override;
    };
};

} // namespace cascade

#endif
