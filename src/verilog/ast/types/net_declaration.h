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

#ifndef CASCADE_SRC_VERILOG_AST_NET_DECLARATION_H
#define CASCADE_SRC_VERILOG_AST_NET_DECLARATION_H

#include "verilog/ast/types/declaration.h"
#include "verilog/ast/types/delay_control.h"
#include "verilog/ast/types/expression.h"
#include "verilog/ast/types/macro.h"
#include "verilog/ast/types/range_expression.h"

namespace cascade {

class NetDeclaration : public Declaration {
  public:
    // Supporting Concepts:
    enum class Type : uint8_t {
      WIRE = 0
    };

    // Constructors:
    NetDeclaration(Attributes* attrs__, Type type__, Identifier* id__, bool signed__);
    NetDeclaration(Attributes* attrs__, Type type__, DelayControl* ctrl__, Identifier* id__, bool signed__, RangeExpression* dim__);
    ~NetDeclaration() override;

    // Node Interface:
    NODE(NetDeclaration)
    NetDeclaration* clone() const override;

    // Get/Set:
    VAL_GET_SET(NetDeclaration, Type, type)
    MAYBE_GET_SET(NetDeclaration, DelayControl, ctrl)
    VAL_GET_SET(NetDeclaration, bool, signed)
    MAYBE_GET_SET(NetDeclaration, RangeExpression, dim)

  private:
    VAL_ATTR(Type, type);
    MAYBE_ATTR(DelayControl, ctrl);
    VAL_ATTR(bool, signed);
    MAYBE_ATTR(RangeExpression, dim);
};

inline NetDeclaration::NetDeclaration(Attributes* attrs__, Type type__, Identifier* id__, bool signed__) : Declaration(Node::Tag::net_declaration) {
  PTR_SETUP(attrs);
  VAL_SETUP(type);
  MAYBE_DEFAULT_SETUP(ctrl);
  PTR_SETUP(id);
  VAL_SETUP(signed);
  MAYBE_DEFAULT_SETUP(dim);
  parent_ = nullptr;
}

inline NetDeclaration::NetDeclaration(Attributes* attrs__, Type type__, DelayControl* ctrl__, Identifier* id__, bool signed__, RangeExpression* dim__) : NetDeclaration(attrs__, type__, id__, signed__) {
  MAYBE_SETUP(ctrl);
  MAYBE_SETUP(dim);
}

inline NetDeclaration::~NetDeclaration() {
  PTR_TEARDOWN(attrs);
  VAL_TEARDOWN(type);
  MAYBE_TEARDOWN(ctrl);
  PTR_TEARDOWN(id);
  VAL_TEARDOWN(signed);
  MAYBE_TEARDOWN(dim);
}

inline NetDeclaration* NetDeclaration::clone() const {
  auto* res = new NetDeclaration(attrs_->clone(), type_, id_->clone(), signed_);
  MAYBE_CLONE(ctrl);
  MAYBE_CLONE(dim);
  return res;
}

} // namespace cascade 

#endif
