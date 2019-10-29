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

#ifndef CASCADE_SRC_TARGET_CORE_AVMM_AVMM_LOGIC_H
#define CASCADE_SRC_TARGET_CORE_AVMM_AVMM_LOGIC_H

#include <unordered_map>
#include <vector>
#include "common/bits.h"
#include "target/core.h"
#include "target/core/avmm/var_table.h"
#include "target/core/avmm/syncbuf.h"
#include "verilog/ast/visitors/visitor.h"

namespace cascade {

class interfacestream;

namespace avmm {

class AvmmCompiler;

class AvmmLogic : public Logic {
  public:
    // Constructors:
    AvmmLogic(Interface* interface, ModuleDeclaration* md, syncbuf& reqs, syncbuf& resps, AvmmCompiler* dc);
    ~AvmmLogic() override;

    // Configuration Methods:
    AvmmLogic& set_input(const Identifier* id, VId vid);
    AvmmLogic& set_state(const Identifier* id, VId vid);
    AvmmLogic& set_output(const Identifier* id, VId vid);
    AvmmLogic& index_tasks();
    AvmmLogic& set_slot(int slot);

    // Configuraton Properties:
    const VarTable32& get_table() const;

    // Core Interface:
    State* get_state() override;
    void set_state(const State* s) override;
    Input* get_input() override;
    void set_input(const Input* i) override;
    void finalize() override;

    void read(VId id, const Bits* b) override;
    void evaluate() override;
    bool there_are_updates() const override;
    void update() override;
    bool there_were_tasks() const override;

    size_t open_loop(VId clk, bool val, size_t itr) override;

    // Optimization Properties:
    bool open_loop_enabled() const;
    const Identifier* open_loop_clock() const;

  private:
    // Compiler State:
    AvmmCompiler* dc_;
    int slot_;

    // Source Management:
    ModuleDeclaration* src_;
    std::vector<const Identifier*> inputs_;
    std::unordered_map<VId, const Identifier*> state_;
    std::vector<std::pair<const Identifier*, VId>> outputs_;
    std::vector<const SystemTaskEnableStatement*> tasks_;

    // Control State:
    bool there_were_tasks_;
    VarTable32 table_;
    std::unordered_map<FId, interfacestream*> streams_;

    // Control Helpers:
    interfacestream* get_stream(FId fd);
    void update_eofs();
    void loop_until_done();
    void handle_outputs();
    void handle_tasks();

    // Indexes system tasks and inserts the identifiers which appear in those
    // tasks into the variable table.
    class Inserter : public Visitor {
      public:
        explicit Inserter(AvmmLogic* de);
        ~Inserter() override = default;
      private:
        AvmmLogic* de_;
        bool in_args_;
        void visit(const Identifier* id) override;
        void visit(const FeofExpression* fe) override;
        void visit(const DebugStatement* ds) override;
        void visit(const FflushStatement* fs) override;
        void visit(const FinishStatement* fs) override;
        void visit(const FseekStatement* fs) override;
        void visit(const GetStatement* gs) override;
        void visit(const PutStatement* ps) override;
        void visit(const RestartStatement* rs) override;
        void visit(const RetargetStatement* rs) override;
        void visit(const SaveStatement* ss) override;
    };

    // Synchronizes the locations in the variable table which correspond to the
    // identifiers which appear in an AST subtree. 
    class Sync : public Visitor {
      public:
        explicit Sync(AvmmLogic* de);
        ~Sync() override = default;
      private:
        AvmmLogic* de_;
        void visit(const Identifier* id) override;
    };
};

} // namespace avmm
} // namespace cascade

#endif
