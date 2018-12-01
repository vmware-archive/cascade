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

#include "src/verilog/ast/visitors/rewriter.h"

#include "src/verilog/ast/ast.h"

namespace cascade {

ArgAssign* Rewriter::rewrite(ArgAssign* aa) {
  aa->accept_exp(this);
  aa->accept_imp(this);
  return aa;
}

Attributes* Rewriter::rewrite(Attributes* a) {
  a->get_as()->accept(this);
  return a;
}

AttrSpec* Rewriter::rewrite(AttrSpec* as) {
  as->accept_lhs(this);
  as->accept_rhs(this);
  return as;
}

CaseGenerateItem* Rewriter::rewrite(CaseGenerateItem* cgi) {
  cgi->get_exprs()->accept(this);
  cgi->accept_block(this);
  return cgi;
}

CaseItem* Rewriter::rewrite(CaseItem* ci) {
  ci->get_exprs()->accept(this);
  ci->accept_stmt(this);
  return ci;
}

Event* Rewriter::rewrite(Event* e) {
  e->accept_expr(this);
  return e;
}

Expression* Rewriter::rewrite(BinaryExpression* be) {
  be->accept_lhs(this);
  be->accept_rhs(this);
  return be;
}

Expression* Rewriter::rewrite(ConditionalExpression* ce) {
  ce->accept_cond(this);
  ce->accept_lhs(this);
  ce->accept_rhs(this);
  return ce;
}

Expression* Rewriter::rewrite(Concatenation* c) {
  c->get_exprs()->accept(this);
  return c;
}

Expression* Rewriter::rewrite(Identifier* id) {
  id->get_ids()->accept(this);
  id->get_dim()->accept(this);
  return id;
}

Expression* Rewriter::rewrite(MultipleConcatenation* mc) {
  mc->accept_expr(this);
  mc->get_concat()->accept(this);
  return mc;
}

Expression* Rewriter::rewrite(Number* n) {
  return n;
}

Expression* Rewriter::rewrite(String* s) {
  return s;
}

Expression* Rewriter::rewrite(RangeExpression* re) {
  re->accept_upper(this);
  re->accept_lower(this);
  return re;
}

Expression* Rewriter::rewrite(UnaryExpression* ue) {
  ue->accept_lhs(this);
  return ue;
}

GenerateBlock* Rewriter::rewrite(GenerateBlock* gb) {
  gb->accept_id(this);
  gb->get_items()->accept(this);
  return gb;
}

Id* Rewriter::rewrite(Id* i) {
  i->accept_isel(this);
  return i;
}

IfGenerateClause* Rewriter::rewrite(IfGenerateClause* igc) {
  igc->get_if()->accept(this);
  igc->accept_then(this);
  return igc;
}

ModuleDeclaration* Rewriter::rewrite(ModuleDeclaration* md) {
  md->get_attrs()->accept(this);
  md->get_id()->accept(this);
  md->get_ports()->accept(this);
  md->get_items()->accept(this);
  return md;
}

ModuleItem* Rewriter::rewrite(AlwaysConstruct* ac) {
  ac->accept_stmt(this);
  return ac;
}

ModuleItem* Rewriter::rewrite(IfGenerateConstruct* igc) {
  igc->get_attrs()->accept(this);
  igc->accept_clauses(this);
  igc->accept_else(this);
  return igc;
}

ModuleItem* Rewriter::rewrite(CaseGenerateConstruct* cgc) {
  cgc->accept_cond(this);
  cgc->get_items()->accept(this);
  return cgc;
}

ModuleItem* Rewriter::rewrite(LoopGenerateConstruct* lgc) {
  lgc->get_init()->accept(this);
  lgc->accept_cond(this);
  lgc->get_update()->accept(this);
  lgc->get_block()->accept(this);
  return lgc;
}

ModuleItem* Rewriter::rewrite(InitialConstruct* ic) {
  ic->get_attrs()->accept(this);
  ic->accept_stmt(this);
  return ic;
}

ModuleItem* Rewriter::rewrite(ContinuousAssign* ca) {
  ca->accept_ctrl(this);
  ca->get_assign()->accept(this);
  return ca;
}

ModuleItem* Rewriter::rewrite(GenvarDeclaration* gd) {
  gd->get_attrs()->accept(this);
  gd->get_id()->accept(this);
  return gd;
}

ModuleItem* Rewriter::rewrite(IntegerDeclaration* id) {
  id->get_attrs()->accept(this);
  id->get_id()->accept(this);
  id->accept_val(this);
  return id;
}

ModuleItem* Rewriter::rewrite(LocalparamDeclaration* ld) {
  ld->get_attrs()->accept(this);
  ld->accept_dim(this);
  ld->get_id()->accept(this);
  ld->accept_val(this);
  return ld;
}

ModuleItem* Rewriter::rewrite(NetDeclaration* nd) {
  nd->get_attrs()->accept(this);
  nd->accept_ctrl(this);
  nd->get_id()->accept(this);
  nd->accept_dim(this);
  return nd;
}

ModuleItem* Rewriter::rewrite(ParameterDeclaration* pd) {
  pd->get_attrs()->accept(this);
  pd->accept_dim(this);
  pd->get_id()->accept(this);
  pd->accept_val(this);
  return pd;
}

ModuleItem* Rewriter::rewrite(RegDeclaration* rd) {
  rd->get_attrs()->accept(this);
  rd->get_id()->accept(this);
  rd->accept_dim(this);
  rd->accept_val(this);
  return rd; 
}

ModuleItem* Rewriter::rewrite(GenerateRegion* gr) {
  gr->get_items()->accept(this);
  return gr;
}

ModuleItem* Rewriter::rewrite(ModuleInstantiation* mi) {
  mi->get_attrs()->accept(this);
  mi->get_mid()->accept(this);
  mi->get_iid()->accept(this);
  mi->accept_range(this);
  mi->get_params()->accept(this);
  mi->get_ports()->accept(this);
  return mi;
}

ModuleItem* Rewriter::rewrite(PortDeclaration* pd) {
  pd->get_attrs()->accept(this);
  pd->get_decl()->accept(this);
  return pd;
}

Statement* Rewriter::rewrite(BlockingAssign* ba) {
  ba->accept_ctrl(this);
  ba->get_assign()->accept(this);
  return ba;
}

Statement* Rewriter::rewrite(NonblockingAssign* na) {
  na->accept_ctrl(this);
  na->get_assign()->accept(this);
  return na;
}

Statement* Rewriter::rewrite(CaseStatement* cs) {
  cs->accept_cond(this);
  cs->get_items()->accept(this);
  return cs;
}

Statement* Rewriter::rewrite(ConditionalStatement* cs) {
  cs->accept_if(this);
  cs->accept_then(this);
  cs->accept_else(this);
  return cs;
}

Statement* Rewriter::rewrite(ForStatement* fs) {
  fs->get_init()->accept(this);
  fs->accept_cond(this);
  fs->get_update()->accept(this);
  fs->accept_stmt(this);
  return fs;
}

Statement* Rewriter::rewrite(ForeverStatement* fs) {
  fs->accept_stmt(this);
  return fs;
}

Statement* Rewriter::rewrite(RepeatStatement* rs) {
  rs->accept_cond(this);
  rs->accept_stmt(this);
  return rs;
}

Statement* Rewriter::rewrite(ParBlock* pb) {
  pb->accept_id(this);
  pb->get_decls()->accept(this);
  pb->get_stmts()->accept(this);
  return pb;
}

Statement* Rewriter::rewrite(SeqBlock* sb) {
  sb->accept_id(this);
  sb->get_decls()->accept(this);
  sb->get_stmts()->accept(this);
  return sb;
}

Statement* Rewriter::rewrite(TimingControlStatement* tcs) {
  tcs->get_ctrl()->accept(this);
  tcs->accept_stmt(this);
  return tcs;
}

Statement* Rewriter::rewrite(DisplayStatement* ds) {
  ds->get_args()->accept(this);
  return ds;
}

Statement* Rewriter::rewrite(FinishStatement* fs) {
  fs->get_arg()->accept(this);
  return fs;
}

Statement* Rewriter::rewrite(WriteStatement* ws) {
  ws->get_args()->accept(this);
  return ws;
}

Statement* Rewriter::rewrite(WaitStatement* ws) {
  ws->accept_cond(this);
  ws->accept_stmt(this);
  return ws;
}

Statement* Rewriter::rewrite(WhileStatement* ws) {
  ws->accept_cond(this);
  ws->accept_stmt(this);
  return ws;
}

TimingControl* Rewriter::rewrite(DelayControl* dc) {
  dc->get_delay()->accept(this);
  return dc;
}

TimingControl* Rewriter::rewrite(EventControl* ec) {
  ec->get_events()->accept(this);
  return ec;
}

VariableAssign* Rewriter::rewrite(VariableAssign* va) {
  va->accept_lhs(this);
  va->accept_rhs(this);
  return va;
}

} // namespace cascade
