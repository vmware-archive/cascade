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

#ifndef CASCADE_SRC_TARGET_CORE_DE10_PASS_MACHINIFY_H
#define CASCADE_SRC_TARGET_CORE_DE10_PASS_MACHINIFY_H

#include <stddef.h>
#include <utility>
#include "src/verilog/ast/visitors/editor.h"
#include "src/verilog/ast/visitors/visitor.h"

namespace cascade {

// Pass 3: 
//        
// Transform always blocks into continuation-passing state machines if they
// contain io tasks.

class Machinify : public Editor {
  public:
    Machinify();
    ~Machinify() override = default;

  private:
    size_t idx_; 
    void edit(AlwaysConstruct* ac) override;

    // Checks whether an always construct contains any i/o statements which
    // require us to trap into the runtime (ie: get() or seek()). 
    struct IoCheck : public Visitor {
      public:
        IoCheck();
        ~IoCheck() override = default;

        bool run(const AlwaysConstruct* ac);

      private:
        bool res_;
        void visit(const GetStatement* gs) override;
        void visit(const PutStatement* ps) override;
    };

    // State Machine Construction Helpers:
    class Generate : public Visitor {
      public:
        Generate(size_t idx);
        ~Generate() = default;

        CaseStatement* run(const Statement* s);

      private:
        CaseStatement* machine_;
        size_t idx_;

        void visit(const BlockingAssign* ba) override;
        void visit(const NonblockingAssign* na) override;
        void visit(const SeqBlock* sb) override;
        void visit(const CaseStatement* cs) override;
        void visit(const ConditionalStatement* cs) override;
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

        Identifier* state_var() const;
        std::pair<size_t, SeqBlock*> current() const;
        void append(const Statement* s);
        void append(SeqBlock* sb, const Statement* s);
        void transition(size_t n);
        void transition(SeqBlock* sb, size_t n);
        void next_state();
    };
};

} // namespace cascade

#endif

