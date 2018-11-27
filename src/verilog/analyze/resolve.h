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

#ifndef CASCADE_SRC_VERILOG_ANALYZE_RESOLVE_H
#define CASCADE_SRC_VERILOG_ANALYZE_RESOLVE_H

#include "src/base/container/vector.h"
#include "src/verilog/ast/visitors/editor.h"

namespace cascade {

// This class is used to associate variable uses with their declarations in the
// AST. This class attaches decorations to the AST to reduce the overhead of
// repeated invocations and relies on up-to-date scope decorations (see
// navigate.h). If the decorations associated with a scope are invalidated or
// if the variables within a scope are transformed in any way, this class must
// invalidate the resolution decorations for any variables that refer to that
// scope before it will work correctly.

class Resolve {
  public:
    // Typedefs:
    typedef typename Vector<const Expression*>::const_iterator use_iterator;

    // Cache Maintenance:
    //
    // Removes dependency references for this node and all of the nodes below
    // it in the AST. 
    void invalidate(const Node* n);

    // Resolution:
    //
    // Returns a pointer to the declaration of this variable. Returns nullptr
    // on failure.
    const Identifier* get_resolution(const Identifier* id);
    // Returns true if this variable contains a slicing subscript. This method
    // is undefined for identifiers which cannot be resolved.
    bool is_slice(const Identifier* id);
    // Returns the fully-qualified name of this variable. For example, eg
    // get_full_id(x) might return root.f[0].x. The caller of this method
    // takes responsibility for the resulting memory.
    Identifier* get_full_id(const Identifier* id);
    // Returns a pointer to the ModuleDeclaration that this identifier appears
    // in. Returns nullptr on failure. For example, for variables that are not
    // part of the AST.
    const ModuleDeclaration* get_parent(const Identifier* id);
    // Returns a pointer to the ModuleDeclaration that this identifier was
    // declared in. Equivalent to calling get_origin(get_resolution(id)).
    const ModuleDeclaration* get_origin(const Identifier* id);

    // Iterators Interface:
    //
    // Iterators over the set of expressions that refer to this variable.  For
    // example, the expression i+1 uses the value of i. Invoking this method on
    // a variable which cannot be resolved is undefined.
    use_iterator use_begin(const Identifier* id);
    use_iterator use_end(const Identifier* id);

  private:
    // Cached accessor helpers:
    const Identifier* cache_resolution(const Identifier* id);
    void cache_uses(Declaration* d);

    // Examines every declaration below this node and inserts an empty use set
    struct InitCacheUses : public Editor {
      ~InitCacheUses() override = default;
      void edit(CaseGenerateConstruct* cgc) override;
      void edit(IfGenerateConstruct* igc) override;
      void edit(LoopGenerateConstruct* lgc) override;
      void edit(GenvarDeclaration* gd) override;
      void edit(IntegerDeclaration* id) override;
      void edit(LocalparamDeclaration* ld) override;
      void edit(NetDeclaration* nd) override;
      void edit(ParameterDeclaration* pd) override;
      void edit(RegDeclaration* rd) override;
      void edit(ModuleInstantiation* mi) override;
    };
    // Populates use sets
    struct CacheUses : public Editor {
      ~CacheUses() override = default;
      void edit(Attributes* as) override;
      void edit(Identifier* i) override;
      void edit(CaseGenerateConstruct* cgc) override;
      void edit(IfGenerateConstruct* igc) override;
      void edit(LoopGenerateConstruct* lgc) override;
      void edit(GenvarDeclaration* gd) override;
      void edit(IntegerDeclaration* id) override;
      void edit(LocalparamDeclaration* ld) override;
      void edit(NetDeclaration* nd) override;
      void edit(ParameterDeclaration* pd) override;
      void edit(RegDeclaration* rd) override;
      void edit(ModuleInstantiation* mi) override;
    };
    // Invalidation cache information:
    struct Invalidate : public Editor {
      ~Invalidate() override = default;
      void edit(Attributes* as) override;
      void edit(Identifier* id) override;
      void edit(CaseGenerateConstruct* cgc) override;
      void edit(IfGenerateConstruct* igc) override;
      void edit(LoopGenerateConstruct* lgc) override;
      void edit(GenvarDeclaration* gd) override;
      void edit(IntegerDeclaration* id) override;
      void edit(LocalparamDeclaration* ld) override;
      void edit(NetDeclaration* nd) override;
      void edit(ParameterDeclaration* pd) override;
      void edit(RegDeclaration* rd) override;
      void edit(ModuleInstantiation* mi) override;
    };
};

} // namespace cascade

#endif
