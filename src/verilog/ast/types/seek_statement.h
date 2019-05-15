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

#ifndef CASCADE_SRC_VERILOG_AST_SEEK_STATEMENT_H
#define CASCADE_SRC_VERILOG_AST_SEEK_STATEMENT_H

#include "verilog/ast/types/macro.h"
#include "verilog/ast/types/expression.h"
#include "verilog/ast/types/identifier.h"
#include "verilog/ast/types/system_task_enable_statement.h"

namespace cascade {

class SeekStatement : public SystemTaskEnableStatement {
  public:
    // Constructors:
    explicit SeekStatement(Identifier* id__, Number* pos__);
    ~SeekStatement() override;

    // Node Interface:
    NODE(SeekStatement)
    SeekStatement* clone() const override;

    // Get/Set:
    PTR_GET_SET(SeekStatement, Identifier, id)
    PTR_GET_SET(SeekStatement, Number, pos)

  private:
    PTR_ATTR(Identifier, id);
    PTR_ATTR(Number, pos);
};

inline SeekStatement::SeekStatement(Identifier* id__, Number* pos__) : SystemTaskEnableStatement(Node::Tag::seek_statement) {
  PTR_SETUP(id);
  PTR_SETUP(pos);
  parent_ = nullptr;
}

inline SeekStatement::~SeekStatement() {
  PTR_TEARDOWN(id);
  PTR_TEARDOWN(pos);
}

inline SeekStatement* SeekStatement::clone() const {
  return new SeekStatement(id_->clone(), pos_->clone());
}

} // namespace cascade 

#endif
