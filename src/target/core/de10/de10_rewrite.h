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
    std::string run(const ModuleDeclaration* md, const De10Logic* de, QuartusServer::Id id);

  private:
    // Records variables which appear in timing control statements
    struct TriggerIndex : public Visitor {
      TriggerIndex();
      ~TriggerIndex() override = default;
      std::map<std::string, const Identifier*> edges_;
      std::map<std::string, const Identifier*> negedges_;
      std::map<std::string, const Identifier*> posedges_;
      void visit(const Event* e) override;
    };

    void emit_port_vars(ModuleDeclaration* res);
    void emit_var_table(ModuleDeclaration* res, const De10Logic* de);
    void emit_shadow_vars(ModuleDeclaration* res, const ModuleDeclaration* md, const De10Logic* de);
    void emit_mask_vars(ModuleDeclaration* res);
    void emit_control_vars(ModuleDeclaration* res);
    void emit_view_vars(ModuleDeclaration* res, const De10Logic* de);
    void emit_trigger_vars(ModuleDeclaration* res, const TriggerIndex* ti);
    void emit_state_vars(ModuleDeclaration* res);

    void emit_update_logic(ModuleDeclaration* res, const De10Logic* de);
    void emit_task_logic(ModuleDeclaration* res, const De10Logic* de);
    void emit_control_logic(ModuleDeclaration* res, const De10Logic* de);
    void emit_var_logic(ModuleDeclaration* res, const ModuleDeclaration* md, const De10Logic* de);
    void emit_trigger_logic(ModuleDeclaration* res, const TriggerIndex* ti);
    void emit_state_logic(ModuleDeclaration* res, const De10Logic* de);
    void emit_output_logic(ModuleDeclaration* res, const De10Logic* de);
          
    void emit_subscript(Identifier* id, size_t idx, size_t n, const std::vector<size_t>& arity) const;
    void emit_slice(Identifier* id, size_t w, size_t i) const;
};

} // namespace cascade

#endif
