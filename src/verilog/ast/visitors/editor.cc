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
  aa->accept_exp(this);
  aa->accept_imp(this);
}

void Editor::edit(Attributes* a) {
  a->accept_as(this);
}

void Editor::edit(AttrSpec* as) {
  as->accept_lhs(this);
  as->accept_rhs(this);
}

void Editor::edit(CaseGenerateItem* cgi) {
  cgi->accept_exprs(this);
  cgi->accept_block(this);
}

void Editor::edit(CaseItem* ci) {
  ci->accept_exprs(this);
  ci->accept_stmt(this);
}

void Editor::edit(Event* e) {
  e->accept_expr(this);
}

void Editor::edit(BinaryExpression* be) { 
  be->accept_lhs(this);
  be->accept_rhs(this);
}

void Editor::edit(ConditionalExpression* ce) {
  ce->accept_cond(this);
  ce->accept_lhs(this);
  ce->accept_rhs(this);
}

void Editor::edit(Concatenation* c) {
  c->accept_exprs(this);
}

void Editor::edit(Identifier* i) {
  i->accept_ids(this);
  i->accept_dim(this);
}

void Editor::edit(MultipleConcatenation* mc) {
  mc->accept_expr(this);
  mc->accept_concat(this);
}

void Editor::edit(Number* n) {
  (void) n;
}

void Editor::edit(String* s) { 
  (void) s;
}

void Editor::edit(RangeExpression* re) {
  re->accept_upper(this);
  re->accept_lower(this);
}

void Editor::edit(UnaryExpression* ue) {
  ue->accept_lhs(this);
}

void Editor::edit(GenerateBlock* gb) {
  gb->accept_id(this);
  gb->accept_items(this);
}

void Editor::edit(Id* i) {
  i->accept_isel(this);
}

void Editor::edit(IfGenerateClause* igc) {
  igc->accept_if(this);
  igc->accept_then(this);
}

void Editor::edit(ModuleDeclaration* md) {
  md->accept_attrs(this);
  md->accept_id(this);
  md->accept_ports(this);
  md->accept_items(this);
}

void Editor::edit(AlwaysConstruct* ac) {
  ac->accept_stmt(this);
}

void Editor::edit(IfGenerateConstruct* igc) {
  igc->accept_attrs(this);
  igc->accept_clauses(this);
  igc->accept_else(this);
}

void Editor::edit(CaseGenerateConstruct* cgc) {
  cgc->accept_cond(this);
  cgc->accept_items(this);
}

void Editor::edit(LoopGenerateConstruct* lgc) {
  lgc->accept_init(this);
  lgc->accept_cond(this);
  lgc->accept_update(this);
  lgc->accept_block(this);
}

void Editor::edit(InitialConstruct* ic) {
  ic->accept_attrs(this);
  ic->accept_stmt(this);
}

void Editor::edit(ContinuousAssign* ca) {
  ca->accept_ctrl(this);
  ca->accept_assign(this);
}

void Editor::edit(GenvarDeclaration* gd) {
  gd->accept_attrs(this);
  gd->accept_id(this);
}

void Editor::edit(IntegerDeclaration* id) {
  id->accept_attrs(this);
  id->accept_id(this); 
  id->accept_val(this);
}

void Editor::edit(LocalparamDeclaration* ld) {
  ld->accept_attrs(this);
  ld->accept_dim(this);
  ld->accept_id(this);
  ld->accept_val(this);
}

void Editor::edit(NetDeclaration* nd) {
  nd->accept_attrs(this);
  nd->accept_ctrl(this);
  nd->accept_id(this);
  nd->accept_dim(this);
}

void Editor::edit(ParameterDeclaration* pd) {
  pd->accept_attrs(this);
  pd->accept_dim(this);
  pd->accept_id(this);
  pd->accept_val(this);
}

void Editor::edit(RegDeclaration* rd) {
  rd->accept_attrs(this);
  rd->accept_id(this);
  rd->accept_dim(this);
  rd->accept_val(this);
}

void Editor::edit(GenerateRegion* gr) {
  gr->accept_items(this);
}

void Editor::edit(ModuleInstantiation* mi) {
  mi->accept_attrs(this);
  mi->accept_mid(this);
  mi->accept_iid(this);
  mi->accept_range(this);
  mi->accept_params(this);
  mi->accept_ports(this);
}

void Editor::edit(PortDeclaration* pd) {
  pd->accept_attrs(this); 
  pd->accept_decl(this); 
}

void Editor::edit(BlockingAssign* ba) {
  ba->accept_ctrl(this);
  ba->accept_assign(this);
}

void Editor::edit(NonblockingAssign* na) {
  na->accept_ctrl(this);
  na->accept_assign(this);
}

void Editor::edit(CaseStatement* cs) {
  cs->accept_cond(this);
  cs->accept_items(this);
}

void Editor::edit(ConditionalStatement* cs) {
  cs->accept_if(this);
  cs->accept_then(this);
  cs->accept_else(this);
}

void Editor::edit(ForStatement* fs) {
  fs->accept_init(this);
  fs->accept_cond(this);
  fs->accept_update(this);
  fs->accept_stmt(this);
}

void Editor::edit(ForeverStatement* fs) {
  fs->accept_stmt(this);
}

void Editor::edit(RepeatStatement* rs) {
  rs->accept_cond(this);
  rs->accept_stmt(this);
}

void Editor::edit(ParBlock* pb) {
  pb->accept_id(this);
  pb->accept_decls(this);
  pb->accept_stmts(this);
}

void Editor::edit(SeqBlock* sb) {
  sb->accept_id(this);
  sb->accept_decls(this);
  sb->accept_stmts(this);
}

void Editor::edit(TimingControlStatement* tcs) {
  tcs->accept_ctrl(this);
  tcs->accept_stmt(this);
}

void Editor::edit(DisplayStatement* ds) {
  ds->accept_args(this);
}

void Editor::edit(FinishStatement* fs) {
  fs->accept_arg(this);
}

void Editor::edit(WriteStatement* ws) {
  ws->accept_args(this);
}

void Editor::edit(WaitStatement* ws) {
  ws->accept_cond(this);
  ws->accept_stmt(this);
}

void Editor::edit(WhileStatement* ws) {
  ws->accept_cond(this);
  ws->accept_stmt(this); 
}

void Editor::edit(DelayControl* dc) {
  dc->accept_delay(this); 
}

void Editor::edit(EventControl* ec) {
  ec->accept_events(this);
}

void Editor::edit(VariableAssign* va) {
  va->accept_lhs(this);
  va->accept_rhs(this);
}

} // namespace cascade

