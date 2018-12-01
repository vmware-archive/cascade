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

#ifndef CASCADE_SRC_VERILOG_AST_MODULE_DECLARATION_H
#define CASCADE_SRC_VERILOG_AST_MODULE_DECLARATION_H

#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include "src/base/container/vector.h"
#include "src/verilog/analyze/indices.h"
#include "src/verilog/ast/types/arg_assign.h"
#include "src/verilog/ast/types/attributes.h"
#include "src/verilog/ast/types/identifier.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/many.h"
#include "src/verilog/ast/types/module_item.h"
#include "src/verilog/ast/types/node.h"
#include "src/verilog/ast/types/scope.h"

namespace cascade {

class ModuleDeclaration : public Node, public Scope {
  public:
    // Constructors:
    ModuleDeclaration(Attributes* attrs__, Identifier* id__, Many<ArgAssign>* ports__, Many<ModuleItem>* items__);
    ~ModuleDeclaration() override;

    // Node Interface:
    NODE(ModuleDeclaration, PTR(attrs), PTR(id), PTR(ports), PTR(items))
    // Get/Set:
    PTR_GET_SET(Attributes*, attrs)
    PTR_GET_SET(Identifier*, id)
    PTR_GET_SET(Many<ArgAssign>*, ports)
    PTR_GET_SET(Many<ModuleItem>*, items)

  private:
    PTR_ATTR(Attributes*, attrs);
    PTR_ATTR(Identifier*, id);
    PTR_ATTR(Many<ArgAssign>*, ports);
    PTR_ATTR(Many<ModuleItem>*, items);

    friend class ModuleInfo;
    DECORATION(size_t, next_update);
    DECORATION(std::unordered_set<const Identifier*>, locals);
    DECORATION(std::unordered_set<const Identifier*>, externals);
    DECORATION(std::unordered_set<const Identifier*>, inputs);
    DECORATION(std::unordered_set<const Identifier*>, outputs);
    DECORATION(std::unordered_set<const Identifier*>, stateful);
    DECORATION(std::unordered_set<const Identifier*>, reads);
    DECORATION(std::unordered_set<const Identifier*>, writes);
    typedef std::unordered_set<const Identifier*, HashId, EqId> ParamSet;
    DECORATION(ParamSet, named_params);
    DECORATION(Vector<const Identifier*>, ordered_params);
    typedef std::unordered_set<const Identifier*, HashId, EqId> PortSet;
    DECORATION(PortSet, named_ports);
    DECORATION(Vector<const Identifier*>, ordered_ports);
    typedef std::unordered_map<const Identifier*, std::unordered_map<const Identifier*, const Expression*>> ConnMap;
    DECORATION(ConnMap, connections);
    typedef std::unordered_map<const Identifier*, const ModuleDeclaration*> ChildMap;
    DECORATION(ChildMap, children);
};

inline ModuleDeclaration::ModuleDeclaration(Attributes* attrs__, Identifier* id__, Many<ArgAssign>* ports__, Many<ModuleItem>* items__) : Node() {
  parent_ = nullptr;
  PTR_SETUP(attrs);
  PTR_SETUP(id);
  PTR_SETUP(ports);
  PTR_SETUP(items);
  next_update_ = 0;
  next_supdate_ = 0;
}

inline ModuleDeclaration::~ModuleDeclaration() {
  PTR_TEARDOWN(attrs);
  PTR_TEARDOWN(id);
  PTR_TEARDOWN(ports);
  PTR_TEARDOWN(items);
}

} // namespace cascade 

#endif
