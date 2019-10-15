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

#ifndef CASCADE_SRC_TARGET_CORE_DE10_PASS_TEXT_MANGLE_H
#define CASCADE_SRC_TARGET_CORE_DE10_PASS_TEXT_MANGLE_H

#include <stddef.h>
#include <vector>
#include "verilog/ast/visitors/builder.h"
#include "verilog/ast/visitors/visitor.h"

namespace cascade {

class De10Logic;

// Pass 1: 
// 
// This pass performs several major text transformations:
// 1. Declarations are deleted.
// 2. Attribute annotations are deleted.
// 3. $feof() expressions are replaced their corresponding vtable entry
// 4. System tasks are transformed into state udpate operations
// 5. Non-blocking assignments are transformed into state update operations
//
class TextMangle : public Builder {
  public:
    TextMangle(const ModuleDeclaration* md, const De10Logic* de);
    ~TextMangle() override = default;

  private:
    const ModuleDeclaration* md_;
    const De10Logic* de_;
    size_t task_index_;

    Attributes* build(const Attributes* as) override;
    ModuleItem* build(const RegDeclaration* rd) override;
    ModuleItem* build(const PortDeclaration* pd) override;
    Expression* build(const FeofExpression* fe) override;
    Statement* build(const NonblockingAssign* na) override;
    Statement* build(const DebugStatement* ds) override;
    Statement* build(const FflushStatement* fs) override;
    Statement* build(const FinishStatement* fs) override;
    Statement* build(const FseekStatement* fs) override;
    Statement* build(const GetStatement* gs) override;
    Statement* build(const PutStatement* ps) override;
    Statement* build(const RestartStatement* rs) override;
    Statement* build(const RetargetStatement* rs) override;
    Statement* build(const SaveStatement* ss) override;

    Expression* get_table_range(const Identifier* r, const Identifier* i);
};

} // namespace cascade

#endif
