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

#ifndef CASCADE_SRC_VERILOG_AST_MODULE_INSTANTIATION_H
#define CASCADE_SRC_VERILOG_AST_MODULE_INSTANTIATION_H

#include "src/verilog/ast/types/arg_assign.h"
#include "src/verilog/ast/types/attributes.h"
#include "src/verilog/ast/types/identifier.h"
#include "src/verilog/ast/types/if_generate_construct.h"
#include "src/verilog/ast/types/instantiation.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/module_declaration.h"
#include "src/verilog/ast/types/range_expression.h"

namespace cascade {

class ModuleInstantiation : public Instantiation {
  public:
    // Constructors:
    ModuleInstantiation(Attributes* attrs__, Identifier* mid__, Identifier* iid__);
    template <typename ParamsItr, typename PortsItr>
    ModuleInstantiation(Attributes* attrs__, Identifier* mid__, Identifier* iid__, RangeExpression* range__, ParamsItr params_begin__, ParamsItr params_end__, PortsItr ports_begin__, PortsItr ports_end__);
    ~ModuleInstantiation() override;

    // Node Interface:
    NODE(ModuleInstantiation)
    ModuleInstantiation* clone() const override;

    // Get/Set:
    PTR_GET_SET(ModuleInstantiation, Attributes, attrs)
    PTR_GET_SET(ModuleInstantiation, Identifier, mid)
    PTR_GET_SET(ModuleInstantiation, Identifier, iid)
    MAYBE_GET_SET(ModuleInstantiation, RangeExpression, range)
    MANY_GET_SET(ModuleInstantiation, ArgAssign, params)
    MANY_GET_SET(ModuleInstantiation, ArgAssign, ports)

    // Convention Interface:
    bool uses_named_params() const;
    bool uses_ordered_params() const;
    bool uses_named_ports() const;
    bool uses_ordered_ports() const;

  private:
    PTR_ATTR(Attributes, attrs);
    PTR_ATTR(Identifier, mid);
    PTR_ATTR(Identifier, iid);
    MAYBE_ATTR(RangeExpression, range);
    MANY_ATTR(ArgAssign, params);
    MANY_ATTR(ArgAssign, ports);

    friend class Elaborate;
    friend class Inline;
    DECORATION(ModuleDeclaration*, inst);
    DECORATION(IfGenerateConstruct*, inline);
};

inline ModuleInstantiation::ModuleInstantiation(Attributes* attrs__, Identifier* mid__, Identifier* iid__) : Instantiation() {
  PTR_SETUP(attrs);
  PTR_SETUP(mid);
  PTR_SETUP(iid);
  MAYBE_DEFAULT_SETUP(range);
  MANY_DEFAULT_SETUP(params);
  MANY_DEFAULT_SETUP(ports);
  parent_ = nullptr;
  inst_ = nullptr;
  inline_ = nullptr;
}

template <typename ParamsItr, typename PortsItr>
inline ModuleInstantiation::ModuleInstantiation(Attributes* attrs__, Identifier* mid__, Identifier* iid__, RangeExpression* range__, ParamsItr params_begin__, ParamsItr params_end__, PortsItr ports_begin__, PortsItr ports_end__) :
    ModuleInstantiation(attrs__, mid__, iid__) {
  MAYBE_SETUP(range);
  MANY_SETUP(params);
  MANY_SETUP(ports);
}

inline ModuleInstantiation::~ModuleInstantiation() {
  PTR_TEARDOWN(attrs);
  PTR_TEARDOWN(mid);
  PTR_TEARDOWN(iid);
  MAYBE_TEARDOWN(range);
  MANY_TEARDOWN(params);
  MANY_TEARDOWN(ports);
  if (inst_ != nullptr) {
    delete inst_;
  }
  if (inline_ != nullptr) {
    delete inline_;
  }
}

inline ModuleInstantiation* ModuleInstantiation::clone() const {
  auto* res = new ModuleInstantiation(attrs_->clone(), mid_->clone(), iid_->clone());
  MAYBE_CLONE(range);
  MANY_CLONE(params);
  MANY_CLONE(ports);
  return res;
}

inline bool ModuleInstantiation::uses_named_params() const {
  return empty_params() || front_params()->is_non_null_exp();
}

inline bool ModuleInstantiation::uses_ordered_params() const {
  return !uses_named_params();
}

inline bool ModuleInstantiation::uses_named_ports() const {
  return empty_ports() || front_ports()->is_non_null_exp();  
}

inline bool ModuleInstantiation::uses_ordered_ports() const {
  return !uses_named_ports();
}

} // namespace cascade 

#endif
