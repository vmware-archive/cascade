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
#include <unordered_map>
#include "common/thread_pool.h"
#include "common/uuid.h"
#include "verilog/ast/ast_fwd.h"
#include "verilog/ast/visitors/visitor.h"

namespace cascade {

class CoreCompiler;
class Engine;
class InterfaceCompiler;
class Runtime;

class Compiler {
  public:
    // Constructors:
    Compiler();
    ~Compiler();

    // Compiler Configuration:
    //
    // These methods are not thread-safe. Invoking these methods after the
    // first invocation of compile() or to override a previously registered
    // compiler is undefined.
    Compiler& set_core_compiler(const std::string& id, CoreCompiler* c);
    Compiler& set_interface_compiler(const std::string& id, InterfaceCompiler* c);
    // These methods are thread-safe. They return either a registered compiler
    // or nullptr if non exists. The thread-safety of method invocations on the 
    // resulting pointers depends on the type of the pointer.
    CoreCompiler* get_core_compiler(const std::string& id);
    InterfaceCompiler* get_interface_compiler(const std::string& id);

    // Compilation Interface:
    // 
    // These methods are thread-safe. Both versions compiles a module
    // declaration into a new engine and returns nullptr if compilation was
    // aborted (either due to premature termination or error). In the case of
    // error, the error reporting interface will indicate what happened.
    // 
    // Attempts to create a new engine. Blocks until completion. 
    Engine* compile(const Uuid& uuid, size_t version, ModuleDeclaration* md);
    // Performs a blocking call to compile to produce a new engine, and replaces
    // the state of the original engine with the result on success. If md contains
    // annotations that specify a second pass compilation, a second thread is started
    // in the background and will use the runtime's interrupt interface to perform a
    // second replacement when it is safe to do so.
    //
    // Users of this method must provide a monotonically increasing version
    // number.  Second pass compilations which return out of order will use
    // this verison number to determine whether replacement is actually
    // necesary. Users must also provide id as a way of identifying md for
    // logging purposes, as compialtion will generally mangle module names.
    void compile_and_replace(Runtime* rt, Engine* e, const Uuid& uuid, size_t& version, ModuleDeclaration* md, const Identifier* id);

    // Error Reporting Interface:
    //
    // These methods are all thread-safe.
    void error(const std::string& s);
    bool error();
    std::string what();

  private:
    // Compilers:
    std::unordered_map<std::string, CoreCompiler*> core_compilers_;
    std::unordered_map<std::string, InterfaceCompiler*> interface_compilers_;

    // JIT State:
    ThreadPool pool_;

    // Error State:
    std::mutex lock_;
    std::string what_;

    // Checks whether a module is a stub: a module with no inputs or outputs,
    // and no runtime-visible side-effects.
    class StubCheck : public Visitor {
      public:
        ~StubCheck() override = default;
        bool check(const ModuleDeclaration* md);
      private:
        bool stub_;
        void visit(const InitialConstruct* ic) override;
        void visit(const FinishStatement* fs) override;
        void visit(const RestartStatement* rs) override;
        void visit(const RetargetStatement* rs) override;
        void visit(const SaveStatement* ss) override;
    };    
};

} // namespace cascade

#endif
