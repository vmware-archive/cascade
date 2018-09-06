// Copyright 2017-2018 VMware, Inc.
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

#ifndef CASCADE_SRC_TARGET_CORE_DE10_DE10_LOGIC_H
#define CASCADE_SRC_TARGET_CORE_DE10_DE10_LOGIC_H

#include <string>
#include <unordered_map>
#include <vector>
#include "src/base/bits/bits.h"
#include "src/target/core.h"
#include "src/verilog/ast/visitors/visitor.h"

namespace cascade {

class De10Logic : public Logic, public Visitor {
  public:
    // Struct Definitions:
    struct VarInfo {
      size_t index;      // Starting index in variable table
      size_t size;       // Size in 32-bit words
      bool materialized; // Was new storage allocated for this variable on the fpga?
      Bits val;          // Variable image in this engine
    };

    // Typedefs:
    typedef std::unordered_map<const Identifier*, VId>::const_iterator map_iterator;
    typedef std::unordered_map<const Identifier*, VarInfo>::const_iterator table_iterator;

    // Constructors:
    De10Logic(Interface* interface, ModuleDeclaration* md, uint8_t* addr);
    ~De10Logic() override;

    // Configuration Methods:
    De10Logic& set_input(const Identifier* id, VId vid);
    De10Logic& set_state(const Identifier* id, VId vid);
    De10Logic& set_output(const Identifier* id, VId vid);

    // Core Interface:
    State* get_state() override;
    void set_state(const State* s) override;
    Input* get_input() override;
    void set_input(const Input* i) override;
    void resync() override;

    void read(VId id, const Bits* b) override;
    void evaluate() override;
    bool there_are_updates() const override;
    void update() override;
    bool there_were_tasks() const override;

    size_t open_loop(VId clk, bool val, size_t itr) override;

    // Iterators over ast id to data plane id conversion:
    map_iterator map_find(const Identifier* id) const;
    map_iterator map_begin() const;
    map_iterator map_end() const;

    // Iterators over variable table:
    table_iterator table_find(const Identifier* id) const;
    table_iterator table_begin() const;
    table_iterator table_end() const;

    // Variable table properties:
    size_t table_size() const;
    size_t live_idx() const;
    size_t there_are_updates_idx() const;
    size_t update_idx() const;
    size_t sys_task_idx() const;
    size_t open_loop_idx() const;

    // Optimization properties:
    bool open_loop_enabled() const;
    const Identifier* open_loop_clock() const;

  private:
    // Source Management:
    ModuleDeclaration* src_;
    size_t next_index_;
    std::unordered_map<const Identifier*, VId> var_map_;
    std::vector<VarInfo*> inputs_;
    std::vector<std::pair<VId, VarInfo*>> outputs_;
    std::vector<const SystemTaskEnableStatement*> tasks_;

    // Connectivity State:
    uint8_t* addr_;

    // Program State:
    std::unordered_map<const Identifier*, VarInfo> var_table_;
    uint32_t task_queue_;

    // Visitor Interface:
    void visit(const DisplayStatement* ds) override;
    void visit(const FinishStatement* fs) override;
    void visit(const WriteStatement* ws) override;

    // Record bookkeeping information for this variable.
    void insert(const Identifier* id, bool materialized);

    // I/O Helpers:
    void read(VarInfo* vi);
    void write(const VarInfo* vi, const Bits* b);
    
    // Evaluate / Update Helpers:
    void handle_outputs();
    void handle_tasks();

    // Printf Logic:
    std::string printf(const Many<Expression>* args);
    const Bits& evaluate(const Expression* e);
};

} // namespace cascade

#endif
