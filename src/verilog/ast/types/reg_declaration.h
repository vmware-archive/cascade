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

#ifndef CASCADE_SRC_VERILOG_AST_REG_DECLARATION_H
#define CASCADE_SRC_VERILOG_AST_REG_DECLARATION_H

#include "src/verilog/ast/types/declaration.h"
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/range_expression.h"

namespace cascade {

class RegDeclaration : public Declaration {
  public:
    // Constructors:
    RegDeclaration(Attributes* attrs__, Identifier* id__, bool signed__);
    RegDeclaration(Attributes* attrs__, Identifier* id__, bool signed__, RangeExpression* dim__, Expression* val__);
    ~RegDeclaration() override;

    // Node Interface:
    NODE(RegDeclaration)
    RegDeclaration* clone() const override;

    // Get/Set:
    VAL_GET_SET(RegDeclaration, bool, signed)
    MAYBE_GET_SET(RegDeclaration, RangeExpression, dim)
    MAYBE_GET_SET(RegDeclaration, Expression, val)

  private:
    VAL_ATTR(bool, signed);
    MAYBE_ATTR(RangeExpression, dim);
    MAYBE_ATTR(Expression, val);
};

inline RegDeclaration::RegDeclaration(Attributes* attrs__, Identifier* id__, bool signed__) : Declaration() {
  PTR_SETUP(attrs);
  PTR_SETUP(id);
  VAL_SETUP(signed);
  MAYBE_DEFAULT_SETUP(dim);
  MAYBE_DEFAULT_SETUP(val);
  parent_ = nullptr;
}

inline RegDeclaration::RegDeclaration(Attributes* attrs__, Identifier* id__, bool signed__, RangeExpression* dim__, Expression* val__) : RegDeclaration(attrs__, id__, signed__) {
  MAYBE_SETUP(dim);
  MAYBE_SETUP(val);
}

inline RegDeclaration::~RegDeclaration() {
  PTR_TEARDOWN(attrs);
  PTR_TEARDOWN(id);
  VAL_TEARDOWN(signed);
  MAYBE_TEARDOWN(dim);
  MAYBE_TEARDOWN(val);
}

inline RegDeclaration* RegDeclaration::clone() const {
  auto* res = new RegDeclaration(attrs_->clone(), id_->clone(), signed_);
  MAYBE_CLONE(dim);
  MAYBE_CLONE(val);
  return res;
}

} // namespace cascade 

#endif
