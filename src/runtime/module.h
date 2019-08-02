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
#include <iosfwd>
#include <stddef.h>
#include <vector>
#include "common/uuid.h"
#include "verilog/ast/visitors/editor.h"
#include "verilog/ast/visitors/visitor.h"

namespace cascade {

class Engine;
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
    Module(const ModuleDeclaration* psrc, Runtime* rt, Module* parent = nullptr);
    ~Module();

    // Hierarchy Interface:
    iterator begin();
    iterator end();

    // Returns the engine associated with this module:
    Engine* engine();
    // Returns the number of modules in this hierarchy:
    size_t size() const;

    // Synchronizes the module hierarchy with changes which have been made to
    // the ast since the previous invocation of synchronize. n is the number of
    // items which have been added to the top-level module in the interim.
    void synchronize(size_t n);
    // Forces a recompilation of the entire module hierarchy.
    void rebuild();
    // Dumps the state of the module hierarchy to an ostream. 
    void save(std::ostream& os);
    // Reads the state of the module hierarchy from an istream. 
    void restart(std::istream& is);

  private:
    // Instantiate modules based on source code
    class Instantiator : public Visitor {
      public:
        explicit Instantiator(Module* ptr);
        ~Instantiator() override = default;
      private:
        void visit(const CaseGenerateConstruct* cgc) override;
        void visit(const IfGenerateConstruct* igc) override;
        void visit(const LoopGenerateConstruct* lgc) override;
        void visit(const ModuleInstantiation* mi) override;
        Module* ptr_;
        std::vector<Module*> instances_;
    };

    // Runtime Handle:
    Runtime* rt_;

    // Hierarchical State:
    const ModuleDeclaration* psrc_;
    Module* parent_;
    std::vector<Module*> children_;

    // Engine State:
    Engine* engine_;
    Uuid uuid_;
    size_t version_;

    // Helper Methods:
    ModuleDeclaration* regenerate_ir_source(size_t ignore);
    void compile_and_replace(size_t ignore);
};

} // namespace cascade

#endif
