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

#ifndef CASCADE_SRC_VERILOG_AST_ERROR_STATEMENT_H
#define CASCADE_SRC_VERILOG_AST_ERROR_STATEMENT_H

#include "verilog/ast/types/expression.h"
#include "verilog/ast/types/macro.h"
#include "verilog/ast/types/system_task_enable_statement.h"

namespace cascade {

class ErrorStatement : public SystemTaskEnableStatement {
  public:
    // Constructors:
    ErrorStatement();
    template <typename ArgsItr>
    ErrorStatement(ArgsItr args_begin__, ArgsItr args_end__);
    ~ErrorStatement() override;

    // Node Interface:
    NODE(ErrorStatement)
    ErrorStatement* clone() const override;

    // Get/Set:
    MANY_GET_SET(ErrorStatement, Expression, args)

  private:
    MANY_ATTR(Expression, args);
};

inline ErrorStatement::ErrorStatement() : SystemTaskEnableStatement(Node::Tag::error_statement) {
  MANY_DEFAULT_SETUP(args);
  parent_ = nullptr;
}

template <typename ArgsItr>
inline ErrorStatement::ErrorStatement(ArgsItr args_begin__, ArgsItr args_end__) : ErrorStatement() {
  MANY_SETUP(args);
}

inline ErrorStatement::~ErrorStatement() {
  MANY_TEARDOWN(args);
}

inline ErrorStatement* ErrorStatement::clone() const {
  auto* res = new ErrorStatement();
  MANY_CLONE(args);
  return res;
}

} // namespace cascade 

#endif
