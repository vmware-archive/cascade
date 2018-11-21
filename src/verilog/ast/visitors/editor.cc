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

#include "src/verilog/ast/visitors/editor.h"

#include "src/verilog/ast/ast.h"

namespace cascade {

void Editor::edit(ArgAssign* aa) {
  aa->get_exp()->accept(this);
  aa->get_imp()->accept(this);
}

void Editor::edit(Attributes* a) {
  a->get_as()->accept(this);
}

void Editor::edit(AttrSpec* as) {
  as->get_lhs()->accept(this);
  as->get_rhs()->accept(this);
}

void Editor::edit(CaseGenerateItem* cgi) {
  cgi->get_exprs()->accept(this);
  cgi->get_block()->accept(this);
}

void Editor::edit(CaseItem* ci) {
  ci->get_exprs()->accept(this);
  ci->get_stmt()->accept(this);
}

void Editor::edit(Event* e) {
  e->get_expr()->accept(this);
}

void Editor::edit(BinaryExpression* be) { 
  be->get_lhs()->accept(this);
  be->get_rhs()->accept(this);
}

void Editor::edit(ConditionalExpression* ce) {
  ce->get_cond()->accept(this);
  ce->get_lhs()->accept(this);
  ce->get_rhs()->accept(this);
}

void Editor::edit(Concatenation* c) {
  c->get_exprs()->accept(this);
}

void Editor::edit(Identifier* i) {
  i->get_ids()->accept(this);
  i->get_dim()->accept(this);
}

void Editor::edit(MultipleConcatenation* mc) {
  mc->get_expr()->accept(this);
  mc->get_concat()->accept(this);
}

void Editor::edit(Number* n) {
  (void) n;
}

void Editor::edit(String* s) { 
  (void) s;
}

void Editor::edit(RangeExpression* re) {
  re->get_upper()->accept(this);
  re->get_lower()->accept(this);
}

void Editor::edit(UnaryExpression* ue) {
  ue->get_lhs()->accept(this);
}

void Editor::edit(GenerateBlock* gb) {
  gb->get_id()->accept(this);
  gb->get_items()->accept(this);
}

void Editor::edit(Id* i) {
  i->get_isel()->accept(this);
}

void Editor::edit(IfGenerateClause* igc) {
  igc->get_if()->accept(this);
  igc->get_then()->accept(this);
}

void Editor::edit(ModuleDeclaration* md) {
  md->get_attrs()->accept(this);
  md->get_id()->accept(this);
  md->get_ports()->accept(this);
  md->get_items()->accept(this);
}

void Editor::edit(AlwaysConstruct* ac) {
  ac->get_stmt()->accept(this);
}

void Editor::edit(IfGenerateConstruct* igc) {
  igc->get_attrs()->accept(this);
  igc->get_clauses()->accept(this);
  igc->get_else()->accept(this);
}

void Editor::edit(CaseGenerateConstruct* cgc) {
  cgc->get_cond()->accept(this);
  cgc->get_items()->accept(this);
}

void Editor::edit(LoopGenerateConstruct* lgc) {
  lgc->get_init()->accept(this);
  lgc->get_cond()->accept(this);
  lgc->get_update()->accept(this);
  lgc->get_block()->accept(this);
}

void Editor::edit(InitialConstruct* ic) {
  ic->get_attrs()->accept(this);
  ic->get_stmt()->accept(this);
}

void Editor::edit(ContinuousAssign* ca) {
  ca->get_ctrl()->accept(this);
  ca->get_assign()->accept(this);
}

void Editor::edit(GenvarDeclaration* gd) {
  gd->get_attrs()->accept(this);
  gd->get_id()->accept(this);
}

void Editor::edit(IntegerDeclaration* id) {
  id->get_attrs()->accept(this);
  id->get_id()->accept(this); 
  id->get_val()->accept(this);
}

void Editor::edit(LocalparamDeclaration* ld) {
  ld->get_attrs()->accept(this);
  ld->get_dim()->accept(this);
  ld->get_id()->accept(this);
  ld->get_val()->accept(this);
}

void Editor::edit(NetDeclaration* nd) {
  nd->get_attrs()->accept(this);
  nd->get_ctrl()->accept(this);
  nd->get_id()->accept(this);
  nd->get_dim()->accept(this);
}

void Editor::edit(ParameterDeclaration* pd) {
  pd->get_attrs()->accept(this);
  pd->get_dim()->accept(this);
  pd->get_id()->accept(this);
  pd->get_val()->accept(this);
}

void Editor::edit(RegDeclaration* rd) {
  rd->get_attrs()->accept(this);
  rd->get_id()->accept(this);
  rd->get_dim()->accept(this);
  rd->get_val()->accept(this);
}

void Editor::edit(GenerateRegion* gr) {
  gr->get_items()->accept(this);
}

void Editor::edit(ModuleInstantiation* mi) {
  mi->get_attrs()->accept(this);
  mi->get_mid()->accept(this);
  mi->get_iid()->accept(this);
  mi->get_range()->accept(this);
  mi->get_params()->accept(this);
  mi->get_ports()->accept(this);
}

void Editor::edit(PortDeclaration* pd) {
  pd->get_attrs()->accept(this); 
  pd->get_decl()->accept(this); 
}

void Editor::edit(BlockingAssign* ba) {
  ba->get_ctrl()->accept(this);
  ba->get_assign()->accept(this);
}

void Editor::edit(NonblockingAssign* na) {
  na->get_ctrl()->accept(this);
  na->get_assign()->accept(this);
}

void Editor::edit(CaseStatement* cs) {
  cs->get_cond()->accept(this);
  cs->get_items()->accept(this);
}

void Editor::edit(ConditionalStatement* cs) {
  cs->get_if()->accept(this);
  cs->get_then()->accept(this);
  cs->get_else()->accept(this);
}

void Editor::edit(ForStatement* fs) {
  fs->get_init()->accept(this);
  fs->get_cond()->accept(this);
  fs->get_update()->accept(this);
  fs->get_stmt()->accept(this);
}

void Editor::edit(ForeverStatement* fs) {
  fs->get_stmt()->accept(this);
}

void Editor::edit(RepeatStatement* rs) {
  rs->get_cond()->accept(this);
  rs->get_stmt()->accept(this);
}

void Editor::edit(ParBlock* pb) {
  pb->get_id()->accept(this);
  pb->get_decls()->accept(this);
  pb->get_stmts()->accept(this);
}

void Editor::edit(SeqBlock* sb) {
  sb->get_id()->accept(this);
  sb->get_decls()->accept(this);
  sb->get_stmts()->accept(this);
}

void Editor::edit(TimingControlStatement* tcs) {
  tcs->get_ctrl()->accept(this);
  tcs->get_stmt()->accept(this);
}

void Editor::edit(DisplayStatement* ds) {
  ds->get_args()->accept(this);
}

void Editor::edit(FinishStatement* fs) {
  fs->get_arg()->accept(this);
}

void Editor::edit(WriteStatement* ws) {
  ws->get_args()->accept(this);
}

void Editor::edit(WaitStatement* ws) {
  ws->get_cond()->accept(this);
  ws->get_stmt()->accept(this);
}

void Editor::edit(WhileStatement* ws) {
  ws->get_cond()->accept(this);
  ws->get_stmt()->accept(this); 
}

void Editor::edit(DelayControl* dc) {
  dc->get_delay()->accept(this); 
}

void Editor::edit(EventControl* ec) {
  ec->get_events()->accept(this);
}

void Editor::edit(VariableAssign* va) {
  va->get_lhs()->accept(this);
  va->get_rhs()->accept(this);
}

} // namespace cascade

