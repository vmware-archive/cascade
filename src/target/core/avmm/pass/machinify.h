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

#ifndef CASCADE_SRC_TARGET_CORE_AVMM_PASS_MACHINIFY_H
#define CASCADE_SRC_TARGET_CORE_AVMM_PASS_MACHINIFY_H

#include <stddef.h>
#include <utility>
#include <vector>
#include "verilog/ast/visitors/visitor.h"

namespace cascade::avmm {

// Pass 2: 
//        
// Removes edge-triggered always blocks from the AST and transforms them into
// continuation-passing state machines which can be combined later on into a
// single monolithic always @(posedge __clk) block. This pass uses system tasks
// as landmarks, but recall that they've been replaced by non-blocking assigns
// to __next_task_id in pass 1.

class Machinify {
  public:
    // State Machine Construction Helpers:
    class Generate : public Visitor {
      public:
        typedef typename std::vector<size_t>::const_iterator task_iterator;

        Generate(size_t idx);
        ~Generate() override = default;

        const ConditionalStatement* text() const;
        size_t name() const;

        size_t final_state() const;
        task_iterator task_begin() const;
        task_iterator task_end() const;

      private:
        friend class Machinify;

        CaseStatement* machine_;
        ConditionalStatement* text_;
        std::vector<size_t> task_states_;
        size_t idx_;

        void run(const EventControl* ec, const Statement* s);
        void visit(const BlockingAssign* ba) override;
        void visit(const NonblockingAssign* na) override;
        void visit(const SeqBlock* sb) override;
        void visit(const CaseStatement* cs) override;
        void visit(const ConditionalStatement* cs) override;

        // Returns the index of the current state and a pointer to the
        // corresponding case item.
        std::pair<size_t, SeqBlock*> current() const;
        // Copies a statement into the current state
        void append(const Statement* s);
        // Copies a statement into a sequential block
        void append(SeqBlock* sb, const Statement* s);
        // Appends a transition to state n to the current state
        void transition(size_t n);
        // Appends a transition to state n to a sequential block 
        void transition(SeqBlock* sb, size_t n);
        // Appends a new state to the state machine. If the machine is
        // non-empty and the current state is not a task state, appends a
        // non-blocking assign task_id <= 0;
        void next_state();
        // Transforms an event into a trigger signal
        Identifier* to_guard(const Event* e) const;
    };

    typedef typename std::vector<Generate>::const_iterator const_iterator;

    ~Machinify();

    void run(ModuleDeclaration* md);
    const_iterator begin() const;
    const_iterator end() const;

  private:
    // Checks whether an always construct contains any task statements which
    // require us to trap into the runtime.
    class TaskCheck : public Visitor {
      public:
        TaskCheck();
        ~TaskCheck() override = default;
        bool run(const Node* n);
      private:
        bool res_;
        void visit(const NonblockingAssign* na) override;
    };

    std::vector<Generate> generators_;
};

} // namespace cascade::avmm

#endif

