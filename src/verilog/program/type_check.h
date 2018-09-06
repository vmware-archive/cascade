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

#ifndef CASCADE_SRC_VERILOG_PROGRAM_TYPE_CHECK_H
#define CASCADE_SRC_VERILOG_PROGRAM_TYPE_CHECK_H

#include <string>
#include "src/base/log/loggable.h"
#include "src/verilog/ast/visitors/visitor.h"

namespace cascade {

class Program;

class TypeCheck : public Loggable, public Visitor {
  public:
    // Constructors:
    TypeCheck(const Program* program);
    ~TypeCheck() override = default;

    // Configuration Interface:
    void deactivate(bool val);
    void warn_unresolved(bool val);
    void local_only(bool val);

    // Pre-elaboration Checking Interface:
    void pre_elaboration_check(const ModuleInstantiation* mi);
    void pre_elaboration_check(const CaseGenerateConstruct* cgc);
    void pre_elaboration_check(const IfGenerateConstruct* igc);
    void pre_elaboration_check(const LoopGenerateConstruct* lgc);

    // Post-Elaboration Checking Interface:
    void post_elaboration_check(const Node* n);

  private:
    // Program Reference:
    const Program* program_;

    // Configuration Flags:
    bool deactivated_;
    bool warn_unresolved_;
    bool local_only_;

    // Location Tracking:
    const Node* outermost_loop_;

    // Logging Helpers:
    void warn(const std::string& s, const Node* n);
    void error(const std::string& s, const Node* n);

    // Visitor Interface:
    void visit(const Identifier* id) override;
    void visit(const GenerateBlock* gb) override;
    void visit(const ModuleDeclaration* md) override;
    void visit(const CaseGenerateConstruct* cgc) override;
    void visit(const IfGenerateConstruct* igc) override;
    void visit(const LoopGenerateConstruct* lgc) override;
    void visit(const InitialConstruct* ic) override;
    void visit(const ContinuousAssign* ca) override;
    void visit(const GenvarDeclaration* id) override;
    void visit(const IntegerDeclaration* id) override;
    void visit(const LocalparamDeclaration* ld) override;
    void visit(const NetDeclaration* nd) override;
    void visit(const ParameterDeclaration* pd) override;
    void visit(const RegDeclaration* rd) override;
    void visit(const ModuleInstantiation* mi) override;
    void visit(const ParBlock* pb) override;
    void visit(const SeqBlock* sb) override;

    // Width Checking Helpers:
    void check_width(const Maybe<RangeExpression>* re);
};

} // namespace cascade

#endif
