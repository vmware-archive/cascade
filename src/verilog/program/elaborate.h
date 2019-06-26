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

#ifndef CASCADE_SRC_VERILOG_PROGRAM_ELABORATE_H
#define CASCADE_SRC_VERILOG_PROGRAM_ELABORATE_H

#include <stddef.h>
#include "common/vector.h"
#include "verilog/ast/visitors/visitor.h"

namespace cascade {

class Program;

class Elaborate : public Visitor {
  public:
    // Constructors:
    explicit Elaborate(const Program* p = nullptr);
    ~Elaborate() override = default;

    // Elaboration Interface:
    ModuleDeclaration* elaborate(ModuleInstantiation* mi);
    GenerateBlock* elaborate(CaseGenerateConstruct* cgc);
    GenerateBlock* elaborate(IfGenerateConstruct* igc);
    Vector<GenerateBlock*>& elaborate(LoopGenerateConstruct* lgc);

    // Query Interface:
    bool is_elaborated(const ModuleInstantiation* mi);
    bool is_elaborated(const CaseGenerateConstruct* cgc);
    bool is_elaborated(const IfGenerateConstruct* igc);
    bool is_elaborated(const LoopGenerateConstruct* lgc);

    ModuleDeclaration* get_elaboration(ModuleInstantiation* mi);
    GenerateBlock* get_elaboration(CaseGenerateConstruct* cgc);
    GenerateBlock* get_elaboration(IfGenerateConstruct* igc);
    Vector<GenerateBlock*>& get_elaboration(LoopGenerateConstruct* lgc);

    const ModuleDeclaration* get_elaboration(const ModuleInstantiation* mi);
    const GenerateBlock* get_elaboration(const CaseGenerateConstruct* cgc);
    const GenerateBlock* get_elaboration(const IfGenerateConstruct* igc);
    const Vector<GenerateBlock*>& get_elaboration(const LoopGenerateConstruct* lgc);

  private:
    // Program Access:
    const Program* program_;

    // Scope Naming State:
    size_t next_name_;
    GenerateConstruct* here_;
    Identifier* get_name(GenerateConstruct* gc);

    // Helper Methods:
    void named_params(ModuleInstantiation* mi);
    void ordered_params(ModuleInstantiation* mi);
    void elaborate(ConditionalGenerateConstruct* cgc, GenerateBlock* b);

    // Visitor Interface:
    void visit(const CaseGenerateConstruct* cgc) override;
    void visit(const IfGenerateConstruct* igc) override;
    void visit(const LoopGenerateConstruct* lgc) override;
};

} // namespace cascade

#endif
