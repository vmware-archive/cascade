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

#include "verilog/ast/visitors/visitor.h"

#include "verilog/ast/ast.h"

namespace cascade {

void Visitor::visit(const ArgAssign* aa) {
  aa->accept_exp(this);
  aa->accept_imp(this);
}

void Visitor::visit(const Attributes* a) {
  a->accept_as(this);
}

void Visitor::visit(const AttrSpec* as) {
  as->accept_lhs(this);
  as->accept_rhs(this);
}

void Visitor::visit(const CaseGenerateItem* cgi) {
  cgi->accept_exprs(this);
  cgi->accept_block(this);
}

void Visitor::visit(const CaseItem* ci) {
  ci->accept_exprs(this);
  ci->accept_stmt(this);
}

void Visitor::visit(const Event* e) {
  e->accept_expr(this);
}

void Visitor::visit(const BinaryExpression* be) { 
  be->accept_lhs(this);
  be->accept_rhs(this);
}

void Visitor::visit(const ConditionalExpression* ce) {
  ce->accept_cond(this);
  ce->accept_lhs(this);
  ce->accept_rhs(this);
}

void Visitor::visit(const FeofExpression* fe) {
  fe->accept_fd(this);
}

void Visitor::visit(const FopenExpression* fe) {
  fe->accept_arg(this);
}

void Visitor::visit(const Concatenation* c) {
  c->accept_exprs(this);
}

void Visitor::visit(const Identifier* i) {
  i->accept_ids(this);
  i->accept_dim(this);
}

void Visitor::visit(const IfGenerateClause* igc) {
  igc->accept_if(this);
  igc->accept_then(this);
}

void Visitor::visit(const MultipleConcatenation* mc) {
  mc->accept_expr(this);
  mc->accept_concat(this);
}

void Visitor::visit(const Number* n) {
  (void) n;
}

void Visitor::visit(const String* s) { 
  (void) s;
}

void Visitor::visit(const RangeExpression* re) {
  re->accept_upper(this);
  re->accept_lower(this);
}

void Visitor::visit(const UnaryExpression* ue) {
  ue->accept_lhs(this);
}

void Visitor::visit(const GenerateBlock* gb) {
  gb->accept_id(this);
  gb->accept_items(this);
}

void Visitor::visit(const Id* i) {
  i->accept_isel(this);
}

void Visitor::visit(const ModuleDeclaration* md) {
  md->accept_attrs(this);
  md->accept_id(this);
  md->accept_ports(this);
  md->accept_items(this);
}

void Visitor::visit(const AlwaysConstruct* ac) {
  ac->accept_stmt(this);
}

void Visitor::visit(const IfGenerateConstruct* igc) {
  igc->accept_attrs(this);
  igc->accept_clauses(this);
  igc->accept_else(this);
}

void Visitor::visit(const CaseGenerateConstruct* cgc) {
  cgc->accept_cond(this);
  cgc->accept_items(this);
}

void Visitor::visit(const LoopGenerateConstruct* lgc) {
  lgc->accept_init(this);
  lgc->accept_cond(this);
  lgc->accept_update(this);
  lgc->accept_block(this);
}

void Visitor::visit(const InitialConstruct* ic) {
  ic->accept_attrs(this);
  ic->accept_stmt(this);
}

void Visitor::visit(const ContinuousAssign* ca) {
  ca->accept_ctrl(this);
  ca->accept_assign(this);
}

void Visitor::visit(const GenvarDeclaration* gd) {
  gd->accept_attrs(this);
  gd->accept_id(this);
}

void Visitor::visit(const IntegerDeclaration* id) {
  id->accept_attrs(this);
  id->accept_id(this); 
  id->accept_val(this);
}

void Visitor::visit(const LocalparamDeclaration* ld) {
  ld->accept_attrs(this);
  ld->accept_dim(this);
  ld->accept_id(this);
  ld->accept_val(this);
}

void Visitor::visit(const NetDeclaration* nd) {
  nd->accept_attrs(this);
  nd->accept_ctrl(this);
  nd->accept_id(this);
  nd->accept_dim(this);
}

void Visitor::visit(const ParameterDeclaration* pd) {
  pd->accept_attrs(this);
  pd->accept_dim(this);
  pd->accept_id(this);
  pd->accept_val(this);
}

void Visitor::visit(const RegDeclaration* rd) {
  rd->accept_attrs(this);
  rd->accept_id(this);
  rd->accept_dim(this);
  rd->accept_val(this);
}

void Visitor::visit(const GenerateRegion* gr) {
  gr->accept_items(this);
}

void Visitor::visit(const ModuleInstantiation* mi) {
  mi->accept_attrs(this);
  mi->accept_mid(this);
  mi->accept_iid(this);
  mi->accept_range(this);
  mi->accept_params(this);
  mi->accept_ports(this);
}

void Visitor::visit(const PortDeclaration* pd) {
  pd->accept_attrs(this); 
  pd->accept_decl(this); 
}

void Visitor::visit(const BlockingAssign* ba) {
  ba->accept_ctrl(this);
  ba->accept_assign(this);
}

void Visitor::visit(const NonblockingAssign* na) {
  na->accept_ctrl(this);
  na->accept_assign(this);
}

void Visitor::visit(const CaseStatement* cs) {
  cs->accept_cond(this);
  cs->accept_items(this);
}

void Visitor::visit(const ConditionalStatement* cs) {
  cs->accept_if(this);
  cs->accept_then(this);
  cs->accept_else(this);
}

void Visitor::visit(const ForStatement* fs) {
  fs->accept_init(this);
  fs->accept_cond(this);
  fs->accept_update(this);
  fs->accept_stmt(this);
}

void Visitor::visit(const ForeverStatement* fs) {
  fs->accept_stmt(this);
}

void Visitor::visit(const RepeatStatement* rs) {
  rs->accept_cond(this);
  rs->accept_stmt(this);
}

void Visitor::visit(const ParBlock* pb) {
  pb->accept_id(this);
  pb->accept_decls(this);
  pb->accept_stmts(this);
}

void Visitor::visit(const SeqBlock* sb) {
  sb->accept_id(this);
  sb->accept_decls(this);
  sb->accept_stmts(this);
}

void Visitor::visit(const TimingControlStatement* tcs) {
  tcs->accept_ctrl(this);
  tcs->accept_stmt(this);
}

void Visitor::visit(const FinishStatement* fs) {
  fs->accept_arg(this);
}

void Visitor::visit(const FseekStatement* fs) {
  fs->accept_fd(this);
  fs->accept_offset(this);
  fs->accept_op(this);
}

void Visitor::visit(const GetStatement* gs) {
  gs->accept_id(this);
  gs->accept_var(this);
}

void Visitor::visit(const PutStatement* ps) {
  ps->accept_id(this);
  ps->accept_var(this);
}

void Visitor::visit(const PutsStatement* ps) {
  ps->accept_fd(this);
  ps->accept_fmt(this);
  ps->accept_expr(this);
}

void Visitor::visit(const RestartStatement* rs) {
  rs->accept_arg(this);
}

void Visitor::visit(const RetargetStatement* rs) {
  rs->accept_arg(this);
}

void Visitor::visit(const SaveStatement* ss) {
  ss->accept_arg(this);
}

void Visitor::visit(const WaitStatement* ws) {
  ws->accept_cond(this);
  ws->accept_stmt(this);
}

void Visitor::visit(const WhileStatement* ws) {
  ws->accept_cond(this);
  ws->accept_stmt(this); 
}

void Visitor::visit(const DelayControl* dc) {
  dc->accept_delay(this); 
}

void Visitor::visit(const EventControl* ec) {
  ec->accept_events(this);
}

void Visitor::visit(const VariableAssign* va) {
  va->accept_lhs(this);
  va->accept_rhs(this);
}

} // namespace cascade
