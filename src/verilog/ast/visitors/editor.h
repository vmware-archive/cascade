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

#ifndef CASCADE_SRC_VERILOG_AST_VISITORS_EDITOR_H
#define CASCADE_SRC_VERILOG_AST_VISITORS_EDITOR_H

#include "src/verilog/ast/ast_fwd.h"

namespace cascade {

// An intrusive visitor. The default implementation of this class performs a
// recursive descent over the AST and provides write access to the nodes that
// it encounters.

struct Editor {
  virtual ~Editor() = default;

  virtual void edit(ArgAssign* aa);
  virtual void edit(Attributes* a);
  virtual void edit(AttrSpec* as);
  virtual void edit(CaseGenerateItem* cgi);
  virtual void edit(CaseItem* ci);
  virtual void edit(Event* e);
  virtual void edit(BinaryExpression* be);
  virtual void edit(ConditionalExpression* ce);
  virtual void edit(EofExpression* ee);
  virtual void edit(FopenExpression* fe);
  virtual void edit(Concatenation* c);
  virtual void edit(Identifier* i);
  virtual void edit(MultipleConcatenation* mc);
  virtual void edit(Number* n);
  virtual void edit(String* s);
  virtual void edit(RangeExpression* re);
  virtual void edit(UnaryExpression* ue);
  virtual void edit(GenerateBlock* gb);
  virtual void edit(Id* i);
  virtual void edit(IfGenerateClause* igc);
  virtual void edit(ModuleDeclaration* md);
  virtual void edit(AlwaysConstruct* ac);
  virtual void edit(IfGenerateConstruct* igc);
  virtual void edit(CaseGenerateConstruct* cgc);
  virtual void edit(LoopGenerateConstruct* lgc);
  virtual void edit(InitialConstruct* ic);
  virtual void edit(ContinuousAssign* ca);
  virtual void edit(GenvarDeclaration* gd);
  virtual void edit(IntegerDeclaration* id);
  virtual void edit(LocalparamDeclaration* ld);
  virtual void edit(NetDeclaration* nd);
  virtual void edit(ParameterDeclaration* pd);
  virtual void edit(RegDeclaration* rd);
  virtual void edit(GenerateRegion* gr);
  virtual void edit(ModuleInstantiation* mi);
  virtual void edit(PortDeclaration* pd);
  virtual void edit(BlockingAssign* ba);
  virtual void edit(NonblockingAssign* na);
  virtual void edit(CaseStatement* cs);
  virtual void edit(ConditionalStatement* cs);
  virtual void edit(ForStatement* fs);
  virtual void edit(ForeverStatement* fs);
  virtual void edit(RepeatStatement* rs);
  virtual void edit(ParBlock* pb);
  virtual void edit(SeqBlock* sb);
  virtual void edit(TimingControlStatement* rcs);
  virtual void edit(DisplayStatement* ds);
  virtual void edit(ErrorStatement* es);
  virtual void edit(FinishStatement* fs);
  virtual void edit(GetStatement* gs);
  virtual void edit(InfoStatement* is);
  virtual void edit(PutStatement* ps);
  virtual void edit(RestartStatement* rs);
  virtual void edit(RetargetStatement* rs);
  virtual void edit(SaveStatement* ss);
  virtual void edit(SeekStatement* ss);
  virtual void edit(WarningStatement* ws);
  virtual void edit(WriteStatement* ws);
  virtual void edit(WaitStatement* ws);
  virtual void edit(WhileStatement* ws);
  virtual void edit(DelayControl* dc);
  virtual void edit(EventControl* ec);
  virtual void edit(VariableAssign* va);
};

} // namespace cascade

#endif

