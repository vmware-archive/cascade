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

#ifndef CASCADE_SRC_VERILOG_AST_VISITORS_REWRITER_H
#define CASCADE_SRC_VERILOG_AST_VISITORS_REWRITER_H

#include "src/verilog/ast/ast_fwd.h"

namespace cascade {

// An intrusive visitor. This class uses the following convention: returning a
// new AST node instead of editting the contents of that node will cause the
// original node to be deleted and replaced by the new one. The default
// implementation of this class performs a recursive descent of the AST and
// leaves nodes unmodified.

struct Rewriter {
  virtual ~Rewriter() = default;

  virtual ArgAssign* rewrite(ArgAssign* aa);
  virtual Attributes* rewrite(Attributes* a);
  virtual AttrSpec* rewrite(AttrSpec* as);
  virtual CaseGenerateItem* rewrite(CaseGenerateItem* cgi);
  virtual CaseItem* rewrite(CaseItem* ci);
  virtual Event* rewrite(Event* e);
  virtual Expression* rewrite(BinaryExpression* be);
  virtual Expression* rewrite(ConditionalExpression* ce);
  virtual Expression* rewrite(FopenExpression* fe);
  virtual Expression* rewrite(Concatenation* c);
  virtual Expression* rewrite(Identifier* i);
  virtual Expression* rewrite(MultipleConcatenation* mc);
  virtual Expression* rewrite(Number* n);
  virtual Expression* rewrite(String* s);
  virtual Expression* rewrite(RangeExpression* re);
  virtual Expression* rewrite(UnaryExpression* ue);
  virtual GenerateBlock* rewrite(GenerateBlock* gb);
  virtual Id* rewrite(Id* i);
  virtual IfGenerateClause* rewrite(IfGenerateClause* igc);
  virtual ModuleDeclaration* rewrite(ModuleDeclaration* md);
  virtual ModuleItem* rewrite(AlwaysConstruct* ac);
  virtual ModuleItem* rewrite(IfGenerateConstruct* igc);
  virtual ModuleItem* rewrite(CaseGenerateConstruct* cgc);
  virtual ModuleItem* rewrite(LoopGenerateConstruct* lgc);
  virtual ModuleItem* rewrite(InitialConstruct* ic);
  virtual ModuleItem* rewrite(ContinuousAssign* ca);
  virtual ModuleItem* rewrite(GenvarDeclaration* gd);
  virtual ModuleItem* rewrite(IntegerDeclaration* id);
  virtual ModuleItem* rewrite(LocalparamDeclaration* ld);
  virtual ModuleItem* rewrite(NetDeclaration* nd);
  virtual ModuleItem* rewrite(ParameterDeclaration* pd);
  virtual ModuleItem* rewrite(RegDeclaration* rd);
  virtual ModuleItem* rewrite(GenerateRegion* gr);
  virtual ModuleItem* rewrite(ModuleInstantiation* mi);
  virtual ModuleItem* rewrite(PortDeclaration* pd);
  virtual Statement* rewrite(BlockingAssign* ba);
  virtual Statement* rewrite(NonblockingAssign* na);
  virtual Statement* rewrite(CaseStatement* cs);
  virtual Statement* rewrite(ConditionalStatement* cs);
  virtual Statement* rewrite(ForStatement* fs);
  virtual Statement* rewrite(ForeverStatement* fs);
  virtual Statement* rewrite(RepeatStatement* rs);
  virtual Statement* rewrite(ParBlock* pb);
  virtual Statement* rewrite(SeqBlock* sb);
  virtual Statement* rewrite(TimingControlStatement* rcs);
  virtual Statement* rewrite(DisplayStatement* ds);
  virtual Statement* rewrite(ErrorStatement* es);
  virtual Statement* rewrite(FinishStatement* fs);
  virtual Statement* rewrite(InfoStatement* is);
  virtual Statement* rewrite(RestartStatement* rs);
  virtual Statement* rewrite(RetargetStatement* rs);
  virtual Statement* rewrite(SaveStatement* ss);
  virtual Statement* rewrite(WarningStatement* ws);
  virtual Statement* rewrite(WriteStatement* ws);
  virtual Statement* rewrite(WaitStatement* ws);
  virtual Statement* rewrite(WhileStatement* ws);
  virtual TimingControl* rewrite(DelayControl* dc);
  virtual TimingControl* rewrite(EventControl* ec);
  virtual VariableAssign* rewrite(VariableAssign* va);
};

} // namespace cascade

#endif
