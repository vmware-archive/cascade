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

#ifndef CASCADE_SRC_VERILOG_AST_WARNING_STATEMENT_H
#define CASCADE_SRC_VERILOG_AST_WARNING_STATEMENT_H

#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/system_task_enable_statement.h"

namespace cascade {

class WarningStatement : public SystemTaskEnableStatement {
  public:
    // Constructors:
    WarningStatement();
    template <typename ArgsItr>
    WarningStatement(ArgsItr args_begin__, ArgsItr args_end__);
    ~WarningStatement() override;

    // Node Interface:
    NODE(WarningStatement)
    WarningStatement* clone() const override;

    // Get/Set:
    MANY_GET_SET(WarningStatement, Expression, args)

  private:
    MANY_ATTR(Expression, args);
};

inline WarningStatement::WarningStatement() : SystemTaskEnableStatement(Node::Tag::warning_statement) {
  MANY_DEFAULT_SETUP(args);
  parent_ = nullptr;
}

template <typename ArgsItr>
inline WarningStatement::WarningStatement(ArgsItr args_begin__, ArgsItr args_end__) : WarningStatement() {
  MANY_SETUP(args);
}

inline WarningStatement::~WarningStatement() {
  MANY_TEARDOWN(args);
}

inline WarningStatement* WarningStatement::clone() const {
  auto* res = new WarningStatement();
  MANY_CLONE(args);
  return res;
}

} // namespace cascade 

#endif
