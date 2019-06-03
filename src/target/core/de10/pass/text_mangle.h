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
// 3. $eof() expressions are replaced their corresponding vtable entry
// 4. System tasks and io tasks are transformed into state udpate operations
// 5. Non-blocking assignments are transformed into state update operations
//
// Note however that further passes use the presence of system tasks and io tasks
// as landmarks for additional code modifications. To account for this, we leave
// dummy ast nodes in their place, and save the mangeld code we generate for later.
// Specifically non-blocking assigns for the form:
//
// - __1 <= n to indicate that this was the n'th io task we replaced
// - __2 <= n to indicate that this was the n'th system task we replaced
// 
// Later on, when these passes have completed, we can make one final pass over the
// ast to replace these dummy nodes with the code we generate here.

class TextMangle : public Builder {
  public:
    TextMangle(const ModuleDeclaration* md, const De10Logic* de);
    ~TextMangle() override = default;

    Statement* get_io(size_t i);
    Statement* get_task(size_t i);

  private:
    const ModuleDeclaration* md_;
    const De10Logic* de_;

    std::vector<Statement*> ios_;
    std::vector<Statement*> tasks_;

    Attributes* build(const Attributes* as) override;
    ModuleItem* build(const RegDeclaration* rd) override;
    ModuleItem* build(const PortDeclaration* pd) override;
    Expression* build(const FeofExpression* fe) override;
    Statement* build(const NonblockingAssign* na) override;
    Statement* build(const DisplayStatement* ds) override;
    Statement* build(const ErrorStatement* es) override;
    Statement* build(const FinishStatement* fs) override;
    Statement* build(const GetStatement* gs) override;
    Statement* build(const InfoStatement* is) override;
    Statement* build(const PutStatement* ps) override;
    Statement* build(const RestartStatement* rs) override;
    Statement* build(const RetargetStatement* rs) override;
    Statement* build(const SaveStatement* ss) override;
    Statement* build(const SeekStatement* ss) override;
    Statement* build(const WarningStatement* ws) override;
    Statement* build(const WriteStatement* ws) override;

    Statement* save_io(const Statement* s);
    Statement* save_task(const Statement* s);

    struct Mangle : public Visitor {
      Mangle(const De10Logic* de, bool within_task, size_t io_idx, size_t task_idx);
      ~Mangle() override = default;

      void visit(const NonblockingAssign* na) override;
      void visit(const Identifier* id) override;
      void visit(const DisplayStatement* ds) override;
      void visit(const ErrorStatement* es) override;
      void visit(const FinishStatement* fs) override;
      void visit(const GetStatement* gs) override;
      void visit(const InfoStatement* is) override;
      void visit(const PutStatement* ps) override;
      void visit(const RestartStatement* rs) override;
      void visit(const RetargetStatement* rs) override;
      void visit(const SaveStatement* ss) override;
      void visit(const SeekStatement* ss) override;
      void visit(const WarningStatement* ws) override;
      void visit(const WriteStatement* ws) override;
      
      void begin_mangle_io();
      void begin_mangle_task();

      Expression* get_table_range(const Identifier* r, const Identifier* i);
    
      const De10Logic* de_;
      bool within_task_;
      size_t io_idx_;
      size_t task_idx_;
      SeqBlock* res_;
    };
};

} // namespace cascade

#endif
