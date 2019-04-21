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

#ifndef CASCADE_SRC_TARGET_CORE_DE10_DE10_REWRITE_H
#define CASCADE_SRC_TARGET_CORE_DE10_DE10_REWRITE_H

#include <map>
#include <string>
#include <vector>
#include "src/target/core/de10/quartus_server.h"
#include "src/verilog/ast/ast_fwd.h"
#include "src/verilog/ast/visitors/builder.h"
#include "src/verilog/ast/visitors/editor.h"
#include "src/verilog/ast/visitors/rewriter.h"
#include "src/verilog/ast/visitors/visitor.h"

namespace cascade {

class De10Logic;

class De10Rewrite {
  public:
    ModuleDeclaration* run(const ModuleDeclaration* md, const De10Logic* de, QuartusServer::Id id);

  private:
    // Code Generation Helpers:
    void append_subscript(Identifier* id, size_t idx, size_t n, const std::vector<size_t>& arity) const;
    void append_slice(Identifier* id, size_t w, size_t i) const;

    // Pass 1: Mangle system tasks but don't don't replace them just yet.
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

    // Pass 2: Delete declarations and attributes, replace non-blocking
    // assigns. Mangle system tasks as well, but don't rewrite them just
    // yet. We need to leave them as is for the next pass.
    class RewriteText : public Builder {
      public:
        RewriteText(const ModuleDeclaration* md, const De10Logic* de);
        ~RewriteText() override = default;

      private:
        const ModuleDeclaration* md_;
        const De10Logic* de_;

        Attributes* build(const Attributes* as) override;
        ModuleItem* build(const RegDeclaration* rd) override;
        ModuleItem* build(const PortDeclaration* pd) override;
        Statement* build(const NonblockingAssign* na) override;
        Expression* get_table_range(const Identifier* r, const Identifier* i);
    };

    // Pass 3: Transform always blocks into continuation-passing state machines
    // if they contain io tasks.
    class Machinify : public Editor {
      public:
        Machinify();
        ~Machinify() override = default;

      private:
        size_t idx_; 
        void edit(AlwaysConstruct* ac) override;

        // State Machine Construction Helpers:
        class Generate : public Visitor {
          public:
            Generate(size_t idx);
            ~Generate() = default;

            CaseStatement* run(const Statement* s);

          private:
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

            CaseStatement* machine_;
            size_t idx_;
        };
    };

    // Pass 4: Now that we're done using system tasks as landmarks and we don't
    // have to worry about introducing new conditionals, go ahead and replace
    // everything from pass 1
    class FinishMangle : public Rewriter {
      public:
        FinishMangle(TaskMangle* tm);
        ~FinishMangle() override = default;

      private:
        TaskMangle* tm_;

        Expression* rewrite(EofExpression* ee) override;
        Statement* rewrite(DisplayStatement* ds) override;
        Statement* rewrite(ErrorStatement* es) override;
        Statement* rewrite(FinishStatement* fs) override;
        Statement* rewrite(GetStatement* gs) override;
        Statement* rewrite(InfoStatement* is) override;
        Statement* rewrite(PutStatement* ps) override;
        Statement* rewrite(RestartStatement* rs) override;
        Statement* rewrite(RetargetStatement* rs) override;
        Statement* rewrite(SaveStatement* ss) override; 
        Statement* rewrite(SeekStatement* ss) override;
        Statement* rewrite(WarningStatement* ws) override;
        Statement* rewrite(WriteStatement* ws) override;
    };
};

} // namespace cascade

#endif
