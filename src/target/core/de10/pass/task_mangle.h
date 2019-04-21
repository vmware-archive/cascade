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

#ifndef CASCADE_SRC_TARGET_CORE_DE10_PASS_TASK_MANGLE_H
#define CASCADE_SRC_TARGET_CORE_DE10_PASS_TASK_MANGLE_H

#include <map>
#include "src/verilog/ast/visitors/visitor.h"

namespace cascade {

class De10Logic;

// Pass 1: 
// 
// Mangle system tasks but don't don't replace them just yet.

class TaskMangle : public Visitor {
  public:
    TaskMangle(const De10Logic* de);
    ~TaskMangle() override = default;

    Node* get(const Node* n);

  private:
    const De10Logic* de_;

    std::map<std::string, Node*> tasks_;
    SeqBlock* t_;
    bool within_task_;
    size_t io_idx_;
    size_t task_idx_;

    void visit(const EofExpression* ee) override;
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
    void finish(const SystemTaskEnableStatement* t);
};

} // namespace cascade

#endif
