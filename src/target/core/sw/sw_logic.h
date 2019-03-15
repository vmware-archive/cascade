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

#ifndef CASCADE_SRC_TARGET_CORE_SW_SW_LOGIC_H
#define CASCADE_SRC_TARGET_CORE_SW_SW_LOGIC_H

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "src/base/bits/bits.h"
#include "src/target/core.h"
#include "src/target/input.h"
#include "src/target/state.h"
#include "src/verilog/ast/visitors/visitor.h"

namespace cascade {

class SwLogic : public Logic, public Visitor {
  public:
    SwLogic(Interface* interface, ModuleDeclaration* md);
    ~SwLogic() override;

    // Configuration Logic:
    SwLogic& set_read(const Identifier* id, VId vid);
    SwLogic& set_write(const Identifier* id, VId vid);
    SwLogic& set_state(const Identifier* id, VId vid);

    // Core Interface:
    State* get_state() override;
    void set_state(const State* s) override;
    Input* get_input() override;
    void set_input(const Input* i) override;
    void finalize() override; 

    void read(VId vid, const Bits* b) override;
    void evaluate() override;
    bool there_are_updates() const override;
    void update() override;
    bool there_were_tasks() const override;

  private:
    // Source Management:
    ModuleDeclaration* src_;
    std::vector<const Identifier*> reads_;
    std::vector<std::pair<const Identifier*, VId>> writes_;
    std::unordered_map<VId, const Identifier*> state_;

    // Control State:
    bool silent_;
    bool there_were_tasks_;
    std::vector<const Node*> active_;
    std::vector<std::tuple<const Identifier*,size_t,int,int>> updates_;
    std::vector<Bits> update_pool_;

    // Scheduling: 
    void schedule_now(const Node* n);
    void schedule_active(const Node* n);
    void notify(const Node* n);

    // Resyncing Helpers:
    void silent_evaluate();

    // Control State:
    uint8_t& get_state(const Statement* s);

    // Visitor Interface:
    void visit(const Event* e) override;
    void visit(const AlwaysConstruct* ac) override;
    void visit(const InitialConstruct* ic) override;
    void visit(const ContinuousAssign* ca) override;
    void visit(const BlockingAssign* ba) override;
    void visit(const NonblockingAssign* na) override;
    void visit(const ParBlock* pb) override;
    void visit(const SeqBlock* sb) override;
    void visit(const CaseStatement* cs) override;
    void visit(const ConditionalStatement* cs) override;
    void visit(const ForStatement* fs) override;
    void visit(const RepeatStatement* rs) override;
    void visit(const WhileStatement* ws) override;
    void visit(const TimingControlStatement* tcs) override;
    void visit(const DisplayStatement* ds) override;
    void visit(const ErrorStatement* es) override;
    void visit(const FinishStatement* fs) override;
    void visit(const InfoStatement* is) override;
    void visit(const RestartStatement* rs) override;
    void visit(const RetargetStatement* rs) override;
    void visit(const SaveStatement* ss) override;
    void visit(const WarningStatement* ws) override;
    void visit(const WriteStatement* ws) override;
    void visit(const WaitStatement* ws) override;
    void visit(const DelayControl* dc) override;
    void visit(const EventControl* ec) override;
    void visit(const VariableAssign* va) override;

    // Debug Printing:
    void log(const std::string& op, const Node* n);
};

} // namespace cascade

#endif
