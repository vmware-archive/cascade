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

#ifndef CASCADE_SRC_VERILOG_AST_VISITORS_BUILDER_H
#define CASCADE_SRC_VERILOG_AST_VISITORS_BUILDER_H

#include "verilog/ast/ast_fwd.h"

namespace cascade {

// A non-intrusive visitor.  This class uses the following convention:
// returning nullptr for an AST node which appears in a combinator (a many or a
// maybe) will result in that node being removed from the AST.  Returning
// nullptr for a node which does not appear in a combinator is undefined.  The
// default implementation of this class performs a recursive descent over the
// AST and returns a clone of the nodes that it encounters. 

struct Builder {
  virtual ~Builder() = default;

  virtual ArgAssign* build(const ArgAssign* aa);
  virtual Attributes* build(const Attributes* a);
  virtual AttrSpec* build(const AttrSpec* as);
  virtual CaseGenerateItem* build(const CaseGenerateItem* cgi);
  virtual CaseItem* build(const CaseItem* ci);
  virtual Event* build(const Event* e);
  virtual Expression* build(const BinaryExpression* be);
  virtual Expression* build(const ConditionalExpression* ce);
  virtual Expression* build(const FeofExpression* fe);
  virtual Expression* build(const FopenExpression* fe);
  virtual Expression* build(const Concatenation* c);
  virtual Expression* build(const Identifier* i);
  virtual Expression* build(const MultipleConcatenation* mc);
  virtual Expression* build(const Number* n);
  virtual Expression* build(const String* s);
  virtual Expression* build(const RangeExpression* re);
  virtual Expression* build(const UnaryExpression* ue);
  virtual GenerateBlock* build(const GenerateBlock* gb);
  virtual Id* build(const Id* i);
  virtual IfGenerateClause* build(const IfGenerateClause* igc);
  virtual ModuleDeclaration* build(const ModuleDeclaration* md);
  virtual ModuleItem* build(const AlwaysConstruct* ac);
  virtual ModuleItem* build(const IfGenerateConstruct* igc);
  virtual ModuleItem* build(const CaseGenerateConstruct* cgc);
  virtual ModuleItem* build(const LoopGenerateConstruct* lgc);
  virtual ModuleItem* build(const InitialConstruct* ic);
  virtual ModuleItem* build(const ContinuousAssign* ca);
  virtual ModuleItem* build(const GenvarDeclaration* gd);
  virtual ModuleItem* build(const IntegerDeclaration* id);
  virtual ModuleItem* build(const LocalparamDeclaration* ld);
  virtual ModuleItem* build(const NetDeclaration* nd);
  virtual ModuleItem* build(const ParameterDeclaration* pd);
  virtual ModuleItem* build(const RegDeclaration* rd);
  virtual ModuleItem* build(const GenerateRegion* gr);
  virtual ModuleItem* build(const ModuleInstantiation* mi);
  virtual ModuleItem* build(const PortDeclaration* pd);
  virtual Statement* build(const BlockingAssign* ba);
  virtual Statement* build(const NonblockingAssign* na);
  virtual Statement* build(const CaseStatement* cs);
  virtual Statement* build(const ConditionalStatement* cs);
  virtual Statement* build(const ForStatement* fs);
  virtual Statement* build(const ForeverStatement* fs);
  virtual Statement* build(const RepeatStatement* rs);
  virtual Statement* build(const ParBlock* pb);
  virtual Statement* build(const SeqBlock* sb);
  virtual Statement* build(const TimingControlStatement* rcs);
  virtual Statement* build(const DisplayStatement* ds);
  virtual Statement* build(const ErrorStatement* es);
  virtual Statement* build(const FinishStatement* fs);
  virtual Statement* build(const GetStatement* gs);
  virtual Statement* build(const InfoStatement* is);
  virtual Statement* build(const PutStatement* ps);
  virtual Statement* build(const RestartStatement* rs);
  virtual Statement* build(const RetargetStatement* rs);
  virtual Statement* build(const SaveStatement* ss);
  virtual Statement* build(const SeekStatement* ss);
  virtual Statement* build(const WarningStatement* ws);
  virtual Statement* build(const WriteStatement* ws);
  virtual Statement* build(const WaitStatement* ws);
  virtual Statement* build(const WhileStatement* ws);
  virtual TimingControl* build(const DelayControl* dc);
  virtual TimingControl* build(const EventControl* ec);
  virtual VariableAssign* build(const VariableAssign* va);
};

} // namespace cascade

#endif
