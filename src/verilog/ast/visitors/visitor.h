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

#ifndef CASCADE_SRC_VERILOG_AST_VISITORS_VISITOR_H
#define CASCADE_SRC_VERILOG_AST_VISITORS_VISITOR_H

#include "src/verilog/ast/ast_fwd.h"

namespace cascade {

// A non-intrusive visitor. The default implementation of this class
// performs a recursive descent over the AST and provides read-only
// access to the nodes that it encounters.

struct Visitor {
  virtual ~Visitor() = default;

  virtual void visit(const ArgAssign* aa);
  virtual void visit(const Attributes* a);
  virtual void visit(const AttrSpec* as);
  virtual void visit(const CaseGenerateItem* cgi);
  virtual void visit(const CaseItem* ci);
  virtual void visit(const Event* e);
  virtual void visit(const BinaryExpression* be);
  virtual void visit(const ConditionalExpression* ce);
  virtual void visit(const EofExpression* ee);
  virtual void visit(const FopenExpression* fe);
  virtual void visit(const Concatenation* c);
  virtual void visit(const Identifier* i);
  virtual void visit(const MultipleConcatenation* mc);
  virtual void visit(const Number* n);
  virtual void visit(const String* s);
  virtual void visit(const RangeExpression* re);
  virtual void visit(const UnaryExpression* ue);
  virtual void visit(const GenerateBlock* gb);
  virtual void visit(const Id* i);
  virtual void visit(const IfGenerateClause* igc);
  virtual void visit(const ModuleDeclaration* md);
  virtual void visit(const AlwaysConstruct* ac);
  virtual void visit(const IfGenerateConstruct* igc);
  virtual void visit(const CaseGenerateConstruct* cgc);
  virtual void visit(const LoopGenerateConstruct* lgc);
  virtual void visit(const InitialConstruct* ic);
  virtual void visit(const ContinuousAssign* ca);
  virtual void visit(const GenvarDeclaration* gd);
  virtual void visit(const IntegerDeclaration* id);
  virtual void visit(const LocalparamDeclaration* ld);
  virtual void visit(const NetDeclaration* nd);
  virtual void visit(const ParameterDeclaration* pd);
  virtual void visit(const RegDeclaration* rd);
  virtual void visit(const GenerateRegion* gr);
  virtual void visit(const ModuleInstantiation* mi);
  virtual void visit(const PortDeclaration* pd);
  virtual void visit(const BlockingAssign* ba);
  virtual void visit(const NonblockingAssign* na);
  virtual void visit(const CaseStatement* cs);
  virtual void visit(const ConditionalStatement* cs);
  virtual void visit(const ForStatement* fs);
  virtual void visit(const ForeverStatement* fs);
  virtual void visit(const RepeatStatement* rs);
  virtual void visit(const ParBlock* pb);
  virtual void visit(const SeqBlock* sb);
  virtual void visit(const TimingControlStatement* rcs);
  virtual void visit(const DisplayStatement* ds);
  virtual void visit(const ErrorStatement* es);
  virtual void visit(const FinishStatement* fs);
  virtual void visit(const FlushStatement* fs);
  virtual void visit(const GetStatement* gs);
  virtual void visit(const InfoStatement* is);
  virtual void visit(const PutStatement* ps);
  virtual void visit(const RestartStatement* rs);
  virtual void visit(const RetargetStatement* rs);
  virtual void visit(const SaveStatement* ss);
  virtual void visit(const SeekStatement* ss);
  virtual void visit(const WarningStatement* ws);
  virtual void visit(const WriteStatement* ws);
  virtual void visit(const WaitStatement* ws);
  virtual void visit(const WhileStatement* ws);
  virtual void visit(const DelayControl* dc);
  virtual void visit(const EventControl* ec);
  virtual void visit(const VariableAssign* va);
};

} // namespace cascade

#endif
