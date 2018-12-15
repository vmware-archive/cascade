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

#ifndef CASCADE_SRC_VERILOG_AST_LOCALPARAM_DECLARATION_H
#define CASCADE_SRC_VERILOG_AST_LOCALPARAM_DECLARATION_H

#include "src/verilog/ast/types/declaration.h"
#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/range_expression.h"

namespace cascade {

class LocalparamDeclaration : public Declaration {
  public:
    // Constructors:
    LocalparamDeclaration(Attributes* attrs__, bool signed__, Identifier* id__, Expression* val__);
    LocalparamDeclaration(Attributes* attrs__, bool signed__, RangeExpression* dim__, Identifier* id__, Expression* val__);
    ~LocalparamDeclaration() override;

    // Node Interface:
    NODE(LocalparamDeclaration)
    LocalparamDeclaration* clone() const override;

    // Get/Set:
    VAL_GET_SET(LocalparamDeclaration, bool, signed)
    MAYBE_GET_SET(LocalparamDeclaration, RangeExpression, dim)
    PTR_GET_SET(LocalparamDeclaration, Expression, val)

  private:
    MAYBE_ATTR(RangeExpression, dim);
    VAL_ATTR(bool, signed);
    PTR_ATTR(Expression, val);
};

inline LocalparamDeclaration::LocalparamDeclaration(Attributes* attrs__, bool signed__, Identifier* id__, Expression* val__) : Declaration() {
  PTR_SETUP(attrs);
  VAL_SETUP(signed);
  MAYBE_DEFAULT_SETUP(dim);
  PTR_SETUP(id);
  PTR_SETUP(val);
  parent_ = nullptr;
}

inline LocalparamDeclaration::LocalparamDeclaration(Attributes* attrs__, bool signed__, RangeExpression* dim__, Identifier* id__, Expression* val__) : LocalparamDeclaration(attrs__, signed__, id__, val__) {
  MAYBE_SETUP(dim);
}

inline LocalparamDeclaration::~LocalparamDeclaration() {
  PTR_TEARDOWN(attrs);
  VAL_TEARDOWN(signed);
  MAYBE_TEARDOWN(dim);
  PTR_TEARDOWN(id);
  PTR_TEARDOWN(val);
}

inline LocalparamDeclaration* LocalparamDeclaration::clone() const {
  auto res = new LocalparamDeclaration(attrs_->clone(), signed_, id_->clone(), val_->clone());
  MAYBE_CLONE(dim);
  return res;
}

} // namespace cascade 

#endif
