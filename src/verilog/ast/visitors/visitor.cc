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

#include "src/verilog/ast/visitors/visitor.h"

#include "src/verilog/ast/ast.h"

namespace cascade {

void Visitor::visit(const ArgAssign* aa) {
  aa->maybe_accept_exp(this);
  aa->maybe_accept_imp(this);
}

void Visitor::visit(const Attributes* a) {
  a->get_as()->accept(this);
}

void Visitor::visit(const AttrSpec* as) {
  as->get_lhs()->accept(this);
  as->get_rhs()->accept(this);
}

void Visitor::visit(const CaseGenerateItem* cgi) {
  cgi->get_exprs()->accept(this);
  cgi->get_block()->accept(this);
}

void Visitor::visit(const CaseItem* ci) {
  ci->get_exprs()->accept(this);
  ci->get_stmt()->accept(this);
}

void Visitor::visit(const Event* e) {
  e->get_expr()->accept(this);
}

void Visitor::visit(const BinaryExpression* be) { 
  be->get_lhs()->accept(this);
  be->get_rhs()->accept(this);
}

void Visitor::visit(const ConditionalExpression* ce) {
  ce->get_cond()->accept(this);
  ce->get_lhs()->accept(this);
  ce->get_rhs()->accept(this);
}

void Visitor::visit(const Concatenation* c) {
  c->get_exprs()->accept(this);
}

void Visitor::visit(const Identifier* i) {
  i->get_ids()->accept(this);
  i->get_dim()->accept(this);
}

void Visitor::visit(const IfGenerateClause* igc) {
  igc->get_if()->accept(this);
  igc->get_then()->accept(this);
}

void Visitor::visit(const MultipleConcatenation* mc) {
  mc->get_expr()->accept(this);
  mc->get_concat()->accept(this);
}

void Visitor::visit(const Number* n) {
  (void) n;
}

void Visitor::visit(const String* s) { 
  (void) s;
}

void Visitor::visit(const RangeExpression* re) {
  re->get_upper()->accept(this);
  re->get_lower()->accept(this);
}

void Visitor::visit(const UnaryExpression* ue) {
  ue->get_lhs()->accept(this);
}

void Visitor::visit(const GenerateBlock* gb) {
  gb->get_id()->accept(this);
  gb->get_items()->accept(this);
}

void Visitor::visit(const Id* i) {
  i->maybe_accept_isel(this);
}

void Visitor::visit(const ModuleDeclaration* md) {
  md->get_attrs()->accept(this);
  md->get_id()->accept(this);
  md->get_ports()->accept(this);
  md->get_items()->accept(this);
}

void Visitor::visit(const AlwaysConstruct* ac) {
  ac->get_stmt()->accept(this);
}

void Visitor::visit(const IfGenerateConstruct* igc) {
  igc->get_attrs()->accept(this);
  igc->get_clauses()->accept(this);
  igc->get_else()->accept(this);
}

void Visitor::visit(const CaseGenerateConstruct* cgc) {
  cgc->get_cond()->accept(this);
  cgc->get_items()->accept(this);
}

void Visitor::visit(const LoopGenerateConstruct* lgc) {
  lgc->get_init()->accept(this);
  lgc->get_cond()->accept(this);
  lgc->get_update()->accept(this);
  lgc->get_block()->accept(this);
}

void Visitor::visit(const InitialConstruct* ic) {
  ic->get_attrs()->accept(this);
  ic->get_stmt()->accept(this);
}

void Visitor::visit(const ContinuousAssign* ca) {
  ca->get_ctrl()->accept(this);
  ca->get_assign()->accept(this);
}

void Visitor::visit(const GenvarDeclaration* gd) {
  gd->get_attrs()->accept(this);
  gd->get_id()->accept(this);
}

void Visitor::visit(const IntegerDeclaration* id) {
  id->get_attrs()->accept(this);
  id->get_id()->accept(this); 
  id->get_val()->accept(this);
}

void Visitor::visit(const LocalparamDeclaration* ld) {
  ld->get_attrs()->accept(this);
  ld->get_dim()->accept(this);
  ld->get_id()->accept(this);
  ld->get_val()->accept(this);
}

void Visitor::visit(const NetDeclaration* nd) {
  nd->get_attrs()->accept(this);
  nd->get_ctrl()->accept(this);
  nd->get_id()->accept(this);
  nd->get_dim()->accept(this);
}

void Visitor::visit(const ParameterDeclaration* pd) {
  pd->get_attrs()->accept(this);
  pd->get_dim()->accept(this);
  pd->get_id()->accept(this);
  pd->get_val()->accept(this);
}

void Visitor::visit(const RegDeclaration* rd) {
  rd->get_attrs()->accept(this);
  rd->get_id()->accept(this);
  rd->get_dim()->accept(this);
  rd->get_val()->accept(this);
}

void Visitor::visit(const GenerateRegion* gr) {
  gr->get_items()->accept(this);
}

void Visitor::visit(const ModuleInstantiation* mi) {
  mi->get_attrs()->accept(this);
  mi->get_mid()->accept(this);
  mi->get_iid()->accept(this);
  mi->get_range()->accept(this);
  mi->get_params()->accept(this);
  mi->get_ports()->accept(this);
}

void Visitor::visit(const PortDeclaration* pd) {
  pd->get_attrs()->accept(this); 
  pd->get_decl()->accept(this); 
}

void Visitor::visit(const BlockingAssign* ba) {
  ba->get_ctrl()->accept(this);
  ba->get_assign()->accept(this);
}

void Visitor::visit(const NonblockingAssign* na) {
  na->get_ctrl()->accept(this);
  na->get_assign()->accept(this);
}

void Visitor::visit(const CaseStatement* cs) {
  cs->get_cond()->accept(this);
  cs->get_items()->accept(this);
}

void Visitor::visit(const ConditionalStatement* cs) {
  cs->get_if()->accept(this);
  cs->get_then()->accept(this);
  cs->get_else()->accept(this);
}

void Visitor::visit(const ForStatement* fs) {
  fs->get_init()->accept(this);
  fs->get_cond()->accept(this);
  fs->get_update()->accept(this);
  fs->get_stmt()->accept(this);
}

void Visitor::visit(const ForeverStatement* fs) {
  fs->get_stmt()->accept(this);
}

void Visitor::visit(const RepeatStatement* rs) {
  rs->get_cond()->accept(this);
  rs->get_stmt()->accept(this);
}

void Visitor::visit(const ParBlock* pb) {
  pb->get_id()->accept(this);
  pb->get_decls()->accept(this);
  pb->get_stmts()->accept(this);
}

void Visitor::visit(const SeqBlock* sb) {
  sb->get_id()->accept(this);
  sb->get_decls()->accept(this);
  sb->get_stmts()->accept(this);
}

void Visitor::visit(const TimingControlStatement* tcs) {
  tcs->get_ctrl()->accept(this);
  tcs->get_stmt()->accept(this);
}

void Visitor::visit(const DisplayStatement* ds) {
  ds->get_args()->accept(this);
}

void Visitor::visit(const FinishStatement* fs) {
  fs->get_arg()->accept(this);
}

void Visitor::visit(const WriteStatement* ws) {
  ws->get_args()->accept(this);
}

void Visitor::visit(const WaitStatement* ws) {
  ws->get_cond()->accept(this);
  ws->get_stmt()->accept(this);
}

void Visitor::visit(const WhileStatement* ws) {
  ws->get_cond()->accept(this);
  ws->get_stmt()->accept(this); 
}

void Visitor::visit(const DelayControl* dc) {
  dc->get_delay()->accept(this); 
}

void Visitor::visit(const EventControl* ec) {
  ec->get_events()->accept(this);
}

void Visitor::visit(const VariableAssign* va) {
  va->get_lhs()->accept(this);
  va->get_rhs()->accept(this);
}

} // namespace cascade
