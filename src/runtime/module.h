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

#ifndef CASCADE_SRC_RUNTIME_MODULE_H
#define CASCADE_SRC_RUNTIME_MODULE_H

#include <forward_list>
#include <stddef.h>
#include <vector>
#include "src/verilog/ast/visitors/editor.h"
#include "src/verilog/ast/visitors/visitor.h"

namespace cascade {

class Compiler;
class DataPlane;
class Engine;
class Isolate;
class Runtime;

class Module {
  public:
    class iterator {
      friend class Module;
      public:
        Module* operator*() const;

        iterator& operator++();
        bool operator==(const iterator& rhs) const;
        bool operator!=(const iterator& rhs) const;

      private:
        std::forward_list<Module*> path_;

        iterator();
        explicit iterator(Module* m);
    };

    // Constructors:
    Module(const ModuleDeclaration* psrc, Runtime* rt, DataPlane* dp, Isolate* isolate, Compiler* compiler);
    ~Module();

    // Runtime Interface:
    // 
    // Synchronizes the module hierarchy with changes which have been made to
    // the ast since the previous invocation of synchronize. n is the number of
    // items which have been added to the top-level module in the interim.
    void synchronize(size_t n);
    // Forces a recompilation of the entire module hierarchy. This method
    // should only be called in a state where synchronize has been invoked and
    // no further changes have been made to the text of the user's program.
    void rebuild();

    // Hierarchy Interface:
    // 
    // Returns the first element in a depth-first traversal of the hierarchy.
    iterator begin();
    // Returns the last element in a depth-first traversal of the hierarchy.
    iterator end();

    // Attribute Interface:
    // 
    // Returns the engine associated with this module.
    Engine* engine();

  private:
    // Instantiation Helper Class:
    struct Instantiator : Visitor {
      explicit Instantiator(Module* ptr);
      ~Instantiator() override = default;

      void visit(const CaseGenerateConstruct* cgc) override;
      void visit(const IfGenerateConstruct* igc) override;
      void visit(const LoopGenerateConstruct* lgc) override;
      void visit(const ModuleInstantiation* mi) override;

      Module* ptr_;
      std::vector<Module*> instances_;
    };

    // Runtime State:
    Runtime* rt_;
    DataPlane* dp_;
    Isolate* isolate_;
    Compiler* compiler_;

    // Implementation State:
    ModuleDeclaration* src_;
    Engine* engine_;

    // Hierarchical State:
    const ModuleDeclaration* psrc_;
    Module* parent_;
    std::vector<Module*> children_;

    // Compilation Helpers:
    //
    // Generates ir source for this module.  Initial statements which appear
    // within the first 'ignore' items of this module are masked as ignored.
    // The caller of this method assumes ownership of the resulting code.
    ModuleDeclaration* regenerate_ir_source(size_t ignore);

    // Constructors:
    Module(const ModuleDeclaration* psrc, Module* parent);
};

} // namespace cascade

#endif
