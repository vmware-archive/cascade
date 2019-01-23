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

#ifndef CASCADE_SRC_TARGET_CORE_DE10_MODULE_BOXER_H
#define CASCADE_SRC_TARGET_CORE_DE10_MODULE_BOXER_H

#include <string>
#include "src/base/stream/indstream.h"
#include "src/runtime/ids.h"
#include "src/target/core/de10/de10_logic.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/ast/visitors/builder.h"
#include "src/verilog/ast/visitors/visitor.h"

namespace cascade {

class ModuleBoxer : public Builder {
  public:
    ~ModuleBoxer() override = default;
    std::string box(MId id, const ModuleDeclaration* md, const De10Logic* de);

  private:
    // Source Management:
    const ModuleDeclaration* md_;
    const De10Logic* de_;

    // System task management:
    size_t task_id_;

    // Builder Interface:
    Attributes* build(const Attributes* as) override; 
    ModuleItem* build(const InitialConstruct* ic) override;
    ModuleItem* build(const RegDeclaration* rd) override;
    ModuleItem* build(const PortDeclaration* pd) override;
    Statement* build(const NonblockingAssign* na) override;
    Statement* build(const DisplayStatement* ds) override;
    Statement* build(const FinishStatement* fs) override;
    Statement* build(const WriteStatement* ws) override;

    // Builder Helpers:
    Expression* get_table_range(const Identifier* r, const Identifier* i);

    // Sys Task Mangling Helper:
    class Mangler : public Visitor {
      public:
        explicit Mangler(const De10Logic* de);
        ~Mangler() override = default;

        template<typename InputItr>
        Statement* mangle(size_t id, InputItr begin, InputItr end);
        Statement* mangle(size_t id, const Node* arg);
      private:
        void init(size_t id);
        void visit(const Identifier* id) override;
        const De10Logic* de_;
        SeqBlock* t_;
    };

    // Code Printing Helpers:
    void emit_variable_table(indstream& os);
    void emit_update_state(indstream& os, ModuleInfo& info);
    void emit_sys_task_state(indstream& os);
    void emit_control_state(indstream& os);
    void emit_view_variables(indstream& os);
    void emit_view_decl(indstream& os, const De10Logic::VarInfo& vinfo);
    void emit_view_init(indstream& os, const De10Logic::VarInfo& vinfo);
    void emit_subscript(indstream& os, size_t idx, size_t n, const std::vector<size_t>& arity);
    void emit_program_logic(indstream& os);
    void emit_update_logic(indstream& os);
    void emit_sys_task_logic(indstream& os);
    void emit_control_logic(indstream& os);
    void emit_variable_table_logic(indstream& os, ModuleInfo& info);
    void emit_slice(indstream& os, size_t w, size_t i);
    void emit_output_logic(indstream& os);
};

template <typename InputItr>
inline Statement* ModuleBoxer::Mangler::mangle(size_t id, InputItr begin, InputItr end) {
  init(id);
  for (; begin != end; ++begin) {
    (*begin)->accept(this);
  }
  return new ConditionalStatement(new Identifier("__live"), t_, new SeqBlock());
}

} // namespace cascade

#endif
