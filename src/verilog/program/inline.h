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

#ifndef CASCADE_SRC_VERILOG_PROGRAM_INLINE_H
#define CASCADE_SRC_VERILOG_PROGRAM_INLINE_H

#include "src/verilog/analyze/module_info.h"
#include "src/verilog/ast/visitors/builder.h"
#include "src/verilog/ast/visitors/editor.h"

namespace cascade {

class Inline : public Editor {
  public:
    // Constructors:
    Inline();
    ~Inline() override = default;

    // Source Generate Interface:
    void inline_source(ModuleDeclaration* md);
    void outline_source(ModuleDeclaration* md);

    // Query Interface:
    bool can_inline(const ModuleDeclaration* md) const;
    bool is_inlined(const ModuleInstantiation* mi);
    const IfGenerateConstruct* get_source(const ModuleInstantiation* mi);

  private:
    class Qualify : public Builder {
      public:
        Qualify();
        ~Qualify() = default;
        Expression* qualify_exp(const Expression* e);
        Identifier* qualify_id(const Identifier* id);
      private:
        Expression* build(const Identifier* id) override;
    };

    // Operational State:
    ModuleInfo* info_;
    bool inline_;

    // Editor Overrides:
    void edit(CaseGenerateConstruct* cgc) override;
    void edit(IfGenerateConstruct* igc) override;
    void edit(LoopGenerateConstruct* lgc) override;
    void edit(ModuleInstantiation* mi) override;

    // Inling Helpers:
    void inline_source(ModuleInstantiation* mi);
    void outline_source(ModuleInstantiation* mi);
};

} // namespace cascade

#endif
