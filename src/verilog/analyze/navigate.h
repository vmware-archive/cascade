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

#ifndef CASCADE_SRC_VERILOG_ANALYZE_NAVIGATE_H
#define CASCADE_SRC_VERILOG_ANALYZE_NAVIGATE_H

#include <unordered_map>
#include "src/base/token/tokenize.h"
#include "src/verilog/ast/types/identifier.h"
#include "src/verilog/ast/types/scope.h"
#include "src/verilog/ast/visitors/visitor.h"

namespace cascade {

// This class is used to navigate the scope hierarchy associated with the AST.
// This class stores decorations in the AST to reduce the overhead of repeated
// invocations. If the state of the AST is ever changed, this class must
// invalidate the scope that contains those changes before it will function
// correctly.

class Navigate : public Visitor {
  public:
    // Iterators:
    class name_iterator {
      friend class Navigate;
      public:
        const Identifier* operator*() const;
        name_iterator& operator++();
        bool operator==(const name_iterator& rhs) const;
        bool operator!=(const name_iterator& rhs) const;
      private:
        Scope::NameMap::const_iterator itr_;
        name_iterator(Scope::NameMap::const_iterator itr);
    };
    class child_iterator {
      friend class Navigate;
      public:
        const Node* operator*() const;
        child_iterator& operator++();
        bool operator==(const child_iterator& rhs) const;
        bool operator!=(const child_iterator& rhs) const;
      private:
        Scope::ChildMap::const_iterator itr_;
        child_iterator(Scope::ChildMap::const_iterator itr);
    };

    // Constructors:
    // 
    // Creates a new scope navigator which is attached to the nearest scope
    // (inclusive) that contains this node.
    Navigate(const Node* n);
    ~Navigate() override = default;

    // Cache Maintenance:
    // 
    // Invalidates the decorations associated with the current scope.  This
    // method is undefined if this navigator is lost.
    void invalidate();

    // Scope Navigation:
    // 
    // Go up on scope level.  This method is undefined if this navigator is
    // lost.
    void up();
    // Attempt to descend into a nested scope with the same name as id. Does
    // nothing and returns false on failure.  This method is undefined if this
    // navigator is lost.
    bool down(const Id* id);
    // Returns true if this navigator has somehow left the scope hierarchy (eg
    // by going up too many times).  This method is undefined if this navigator
    // is lost.
    bool lost() const;
    // Returns true if this navigator is currently positioned at the top of the
    // scope hierarchy.
    bool root() const;
    // Returns a pointer to the current scope. This method is undefined if this
    // navigator is lost.
    const Node* where() const; 
    // Returns the name of this scope or nullptr if this is the root scope
    // level of a module declaration. This method is undefined if this
    // navigator is lost.
    const Identifier* name() const; 

    // Scope Exploration:
    //
    // Returns a pointer to the declaration of an identifier with the same name
    // as this id in this scope, or nullptr on failure.
    const Identifier* find_name(const Id* id); 
    // Returns a pointer to the declaration of an identifier with the same name
    // as this id in this scope which is not the same as id, or nullptr on
    // failure.
    const Identifier* find_duplicate_name(const Id* id);
    // Returns a pointer to a nested scope with the same name as this id, or 
    // nullptr on failure.
    const Node* find_child(const Id* id);
    // Identical to find_child(), but ignores subscripts. If the argument to
    // this function matches one or more subscripted scopes, it returns an
    // arbitrary scope.
    const Node* find_child_ignore_subscripts(const Id* id);

    // Iterators:
    // 
    // Iterators over the names that are declared in this scope.
    name_iterator name_begin() const;
    name_iterator name_end() const;
    // Iterator over the scopes that are nested within this scope.
    child_iterator child_begin() const;
    child_iterator child_end() const;

  private:
    // Scope Pointer:
    Node* where_;

    // Scope Navigation Helpers:
    bool boundary_check() const;
    bool location_check() const;

    // Caching Helpers:
    void cache_name(const Identifier* id);

    // Visitor Interface:
    void visit(const GenerateBlock* gb) override;
    void visit(const CaseGenerateConstruct* cgc) override;
    void visit(const IfGenerateConstruct* igc) override;
    void visit(const LoopGenerateConstruct* lgc) override;
    void visit(const GenvarDeclaration* gd) override;
    void visit(const IntegerDeclaration* id) override;
    void visit(const LocalparamDeclaration* ld) override;
    void visit(const NetDeclaration* nd) override;
    void visit(const ParameterDeclaration* pd) override;
    void visit(const RegDeclaration* rd) override;
    void visit(const ModuleInstantiation* mi) override;
    void visit(const ParBlock* pb) override;
    void visit(const SeqBlock* sb) override;

    // Helper Methods:
    void refresh();
};

} // namespace cascade

#endif
