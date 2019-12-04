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

#ifndef CASCADE_SRC_VERILOG_AST_TYPES_FFLUSH_STATEMENT_H
#define CASCADE_SRC_VERILOG_AST_TYPES_FFLUSH_STATEMENT_H

#include "verilog/ast/types/expression.h"
#include "verilog/ast/types/macro.h"
#include "verilog/ast/types/system_task_enable_statement.h"

namespace cascade {

class FflushStatement : public SystemTaskEnableStatement {
  public:
    // Constructors:
    explicit FflushStatement(Expression* fd__);
    ~FflushStatement() override;

    // Node Interface:
    NODE(FflushStatement)
    FflushStatement* clone() const override;

    // Get/Set:
    PTR_GET_SET(FflushStatement, Expression, fd)

  private:
    PTR_ATTR(Expression, fd);
};

inline FflushStatement::FflushStatement(Expression* fd__) : SystemTaskEnableStatement(Node::Tag::fflush_statement) {
  PTR_SETUP(fd);
  parent_ = nullptr;
}

inline FflushStatement::~FflushStatement() {
  PTR_TEARDOWN(fd);
}

inline FflushStatement* FflushStatement::clone() const {
  return new FflushStatement(fd_->clone());
}

} // namespace cascade 

#endif
