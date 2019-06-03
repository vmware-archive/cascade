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

#ifndef CASCADE_SRC_VERILOG_PRINT_PRINTER_H
#define CASCADE_SRC_VERILOG_PRINT_PRINTER_H

#include <iostream>
#include <string>
#include <stdint.h>
#include "base/stream/indstream.h"
#include "verilog/ast/visitors/visitor.h"
#include "verilog/print/color.h"

namespace cascade {

class Printer : public Visitor {
  public:
    explicit Printer(std::ostream& os);
    ~Printer() override = default;

    void visit(const ArgAssign* aa) override;
    void visit(const Attributes* a) override;
    void visit(const AttrSpec* as) override;
    void visit(const CaseGenerateItem* cgi) override;
    void visit(const CaseItem* ci) override;
    void visit(const Event* e) override;
    void visit(const BinaryExpression* be) override;
    void visit(const ConditionalExpression* ce) override;
    void visit(const FeofExpression* fe) override;
    void visit(const FopenExpression* fe) override;
    void visit(const Concatenation* c) override;
    void visit(const Identifier* i) override;
    void visit(const MultipleConcatenation* mc) override;
    void visit(const Number* n) override; 
    void visit(const String* s) override;
    void visit(const RangeExpression* re) override;
    void visit(const UnaryExpression* ue) override;
    void visit(const GenerateBlock* gb) override;
    void visit(const Id* i) override;
    void visit(const IfGenerateClause* igc) override;
    void visit(const ModuleDeclaration* md) override;
    void visit(const AlwaysConstruct* ac) override;
    void visit(const IfGenerateConstruct* igc) override;
    void visit(const CaseGenerateConstruct* cgc) override;
    void visit(const LoopGenerateConstruct* lgc) override;
    void visit(const InitialConstruct* ic) override;
    void visit(const ContinuousAssign* ca) override;
    void visit(const GenvarDeclaration* gd) override;
    void visit(const IntegerDeclaration* id) override;
    void visit(const LocalparamDeclaration* ld) override;
    void visit(const NetDeclaration* nd) override;
    void visit(const ParameterDeclaration* pd) override;
    void visit(const RegDeclaration* rd) override;
    void visit(const GenerateRegion* gr) override;
    void visit(const ModuleInstantiation* mi) override;
    void visit(const PortDeclaration* pd) override;
    void visit(const BlockingAssign* ba) override;
    void visit(const NonblockingAssign* na) override;
    void visit(const CaseStatement* cs) override;
    void visit(const ConditionalStatement* cs) override;
    void visit(const ForStatement* fs) override;
    void visit(const ForeverStatement* fs) override;
    void visit(const RepeatStatement* rs) override;
    void visit(const ParBlock* pb) override;
    void visit(const SeqBlock* sb) override;
    void visit(const TimingControlStatement* tcs) override;
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
    void visit(const WaitStatement* ws) override;
    void visit(const WhileStatement* ws) override;
    void visit(const DelayControl* dc) override;
    void visit(const EventControl* ec) override;
    void visit(const VariableAssign* va) override;

    Printer& operator<<(Color c);
    Printer& operator<<(Node* n);
    Printer& operator<<(const Node* n);
    Printer& operator<<(uint64_t n);
    Printer& operator<<(const std::string& s);

  protected:
    virtual std::string reset() = 0;
    virtual std::string red() = 0;
    virtual std::string green() = 0;
    virtual std::string yellow() = 0;
    virtual std::string blue() = 0;
    virtual std::string grey() = 0;

  private:
    indstream os_;
};

} // namespace cascade

#endif
