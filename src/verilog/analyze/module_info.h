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

#ifndef CASCADE_SRC_VERILOG_ANALYZE_MODULE_INFO_H
#define CASCADE_SRC_VERILOG_ANALYZE_MODULE_INFO_H

#include <unordered_map>
#include <unordered_set>
#include "src/base/container/vector.h"
#include "src/verilog/analyze/indices.h"
#include "src/verilog/ast/visitors/visitor.h"

namespace cascade {

// This class is used to track the high-level properties of AST modules. It can
// be applied to either module declarations or modules which have been
// instantiated into the module hierarchy. This class stores decorations in the
// AST to reduce the overhead of repeated invocations and depends on up-to-date
// scope and resolution information. If the AST decorations associated with
// either are ever invalidated or the structure of this module is ever changed,
// this class must invalidate that module before it will work correctly.
        
class ModuleInfo : public Visitor {
  public:
    // Constructors:
    explicit ModuleInfo(const ModuleDeclaration* md);
    // Does nothing. No state is stored in this class.
    ~ModuleInfo() override = default;

    // Cache Maintenance:
    // 
    // Erases any decroations associated with this module.
    void invalidate();

    // Hierarchical Properties:
    //
    // Returns true if this is a stand-alone declaration.
    bool is_declaration();
    // Returns true if this module has been instantiated.
    bool is_instantiated();
    // Returns nullptr for module declarations, id for instantiations.
    const Identifier* id();

    // Variable Queries:
    //
    // Returns true if this variable resolves to a declaration in this module.
    // Note that !is_local(x) =/= is_external(x).
    bool is_local(const Identifier* id);
    // Returns true if this a local variable which was declared as an input port.
    bool is_input(const Identifier* id);
    // Returns true if this a local variable which was declared as an output
    // port.
    bool is_output(const Identifier* id);
    // Returns true if this a local variable which is the target of a
    // nonblocking assign or a file descriptor.
    bool is_stateful(const Identifier* id);
    // Returns true if variable resolves to a declaration outside of this
    // module.  Note that !is_external(x) =/= is_local(x).
    bool is_external(const Identifier* id);
    // Returns true if this variable is read by another module, either through
    // a module instantiation or a hierarchical dereference in a location other
    // than the lhs of an assignment. 
    bool is_read(const Identifier* id);
    // Returns true if this variable is written by another module, either
    // through a module instantiation or a hierarchical dereference on the lhs
    // of an assignment. 
    bool is_write(const Identifier* id);
    // Returns true if this variable resolves to a module instantiation in this
    // module.
    bool is_child(const Identifier* id);

    // Variable Indices:
    //
    // Returns the set of variables for which is_local(x) returns true.
    const std::unordered_set<const Identifier*>& locals(); 
    // Returns the set of variables for which is_input(x) returns true.
    const std::unordered_set<const Identifier*>& inputs(); 
    // Returns the set of variables for which is_output(x) returns true.
    const std::unordered_set<const Identifier*>& outputs(); 
    // Returns the set of variables for which is_stateful(x) returns true.
    const std::unordered_set<const Identifier*>& stateful(); 
    // Returns the set of variables for which is_external(x) returns true.
    const std::unordered_set<const Identifier*>& externals(); 
    // Returns the set of variables for which is_read(x) returns true.
    const std::unordered_set<const Identifier*>& reads(); 
    // Returns the set of variables for which is_write(x) returns true.
    const std::unordered_set<const Identifier*>& writes(); 
    // Returns a map from variables for which is_child(x) returns true to the
    // corresponding instantiation of those modules.
    const std::unordered_map<const Identifier*, const ModuleDeclaration*>& children();

    // Parameter Indices:
    //
    // Returns the set of local variables in this module that were declared as
    // parameters.  Note that this set is indexed by name, rather than pointer
    // value. 
    const std::unordered_set<const Identifier*, HashId, EqId>& named_params();
    // Returns the set of local variables in this module that were declared as
    // parameters in the order of their declaration.
    const Vector<const Identifier*>& ordered_params();

    // Port Indices:
    //
    // Returns the set of local variables in this module that were declared as
    // ports.  Note that this set is indexed by name, rather than pointer
    // value. 
    const std::unordered_set<const Identifier*, HashId, EqId>& named_ports();
    // Returns the set of local variables in this module that were declared as
    // ports in the order of their declaration.
    const Vector<const Identifier*>& ordered_ports();

    // Connection Indices:
    //
    // Returns a map from variables for which is_child(x) returns true to a map
    // from the target port to the value this module attaches to it.
    const std::unordered_map<const Identifier*, std::unordered_map<const Identifier*, const Expression*>>& connections();

  private:
    ModuleDeclaration* md_;
    bool lhs_;

    // Lazy Computation Helpers:
    void named_parent_conn(const ModuleInstantiation* mi, const PortDeclaration* pd);
    void ordered_parent_conn(const ModuleInstantiation* mi, const PortDeclaration* pd, size_t idx);
    void named_child_conns(const ModuleInstantiation* mi);
    void ordered_child_conns(const ModuleInstantiation* mi);
    void named_external_conn(const ModuleInstantiation* mi, const ArgAssign* aa, const Identifier* id);
    void ordered_external_conn(const ModuleInstantiation* mi, const ArgAssign* aa, const Identifier* id);
    void record_local_read(const Identifier* id);
    void record_external_read(const Identifier* id);
    void record_local_write(const Identifier* id);
    void record_external_write(const Identifier* id);
    void record_external_use(const Identifier* id);

    // Visitor Interface:
    void visit(const Attributes* as) override;
    void visit(const Identifier* i) override;
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
    void visit(const PortDeclaration* pd) override;
    void visit(const NonblockingAssign* na) override;
    void visit(const VariableAssign* va) override;

    // Cache Maintenance Helpers:
    void refresh();
};

} // namespace cascade 

#endif
