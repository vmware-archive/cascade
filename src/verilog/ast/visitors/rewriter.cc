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

#include "verilog/ast/visitors/rewriter.h"

#include "verilog/ast/ast.h"

namespace cascade {

ArgAssign* Rewriter::rewrite(ArgAssign* aa) {
  aa->accept_exp(this);
  aa->accept_imp(this);
  return aa;
}

Attributes* Rewriter::rewrite(Attributes* a) {
  a->accept_as(this);
  return a;
}

AttrSpec* Rewriter::rewrite(AttrSpec* as) {
  as->accept_lhs(this);
  as->accept_rhs(this);
  return as;
}

CaseGenerateItem* Rewriter::rewrite(CaseGenerateItem* cgi) {
  cgi->accept_exprs(this);
  cgi->accept_block(this);
  return cgi;
}

CaseItem* Rewriter::rewrite(CaseItem* ci) {
  ci->accept_exprs(this);
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

Expression* Rewriter::rewrite(FeofExpression* fe) {
  fe->accept_fd(this);
  return fe;
}

Expression* Rewriter::rewrite(FopenExpression* fe) {
  fe->accept_path(this);
  return fe;
}

Expression* Rewriter::rewrite(Concatenation* c) {
  c->accept_exprs(this);
  return c;
}

Expression* Rewriter::rewrite(Identifier* id) {
  id->accept_ids(this);
  id->accept_dim(this);
  return id;
}

Expression* Rewriter::rewrite(MultipleConcatenation* mc) {
  mc->accept_expr(this);
  mc->accept_concat(this);
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
  gb->accept_items(this);
  return gb;
}

Id* Rewriter::rewrite(Id* i) {
  i->accept_isel(this);
  return i;
}

IfGenerateClause* Rewriter::rewrite(IfGenerateClause* igc) {
  igc->accept_if(this);
  igc->accept_then(this);
  return igc;
}

ModuleDeclaration* Rewriter::rewrite(ModuleDeclaration* md) {
  md->accept_attrs(this);
  md->accept_id(this);
  md->accept_ports(this);
  md->accept_items(this);
  return md;
}

ModuleItem* Rewriter::rewrite(AlwaysConstruct* ac) {
  ac->accept_stmt(this);
  return ac;
}

ModuleItem* Rewriter::rewrite(IfGenerateConstruct* igc) {
  igc->accept_attrs(this);
  igc->accept_clauses(this);
  igc->accept_else(this);
  return igc;
}

ModuleItem* Rewriter::rewrite(CaseGenerateConstruct* cgc) {
  cgc->accept_cond(this);
  cgc->accept_items(this);
  return cgc;
}

ModuleItem* Rewriter::rewrite(LoopGenerateConstruct* lgc) {
  lgc->accept_init(this);
  lgc->accept_cond(this);
  lgc->accept_update(this);
  lgc->accept_block(this);
  return lgc;
}

ModuleItem* Rewriter::rewrite(InitialConstruct* ic) {
  ic->accept_attrs(this);
  ic->accept_stmt(this);
  return ic;
}

ModuleItem* Rewriter::rewrite(ContinuousAssign* ca) {
  ca->accept_lhs(this);
  ca->accept_rhs(this);
  return ca;
}

ModuleItem* Rewriter::rewrite(GenvarDeclaration* gd) {
  gd->accept_attrs(this);
  gd->accept_id(this);
  return gd;
}

ModuleItem* Rewriter::rewrite(LocalparamDeclaration* ld) {
  ld->accept_attrs(this);
  ld->accept_dim(this);
  ld->accept_id(this);
  ld->accept_val(this);
  return ld;
}

ModuleItem* Rewriter::rewrite(NetDeclaration* nd) {
  nd->accept_attrs(this);
  nd->accept_id(this);
  nd->accept_dim(this);
  return nd;
}

ModuleItem* Rewriter::rewrite(ParameterDeclaration* pd) {
  pd->accept_attrs(this);
  pd->accept_dim(this);
  pd->accept_id(this);
  pd->accept_val(this);
  return pd;
}

ModuleItem* Rewriter::rewrite(RegDeclaration* rd) {
  rd->accept_attrs(this);
  rd->accept_id(this);
  rd->accept_dim(this);
  rd->accept_val(this);
  return rd; 
}

ModuleItem* Rewriter::rewrite(GenerateRegion* gr) {
  gr->accept_items(this);
  return gr;
}

ModuleItem* Rewriter::rewrite(ModuleInstantiation* mi) {
  mi->accept_attrs(this);
  mi->accept_mid(this);
  mi->accept_iid(this);
  mi->accept_range(this);
  mi->accept_params(this);
  mi->accept_ports(this);
  return mi;
}

ModuleItem* Rewriter::rewrite(PortDeclaration* pd) {
  pd->accept_attrs(this);
  pd->accept_decl(this);
  return pd;
}

Statement* Rewriter::rewrite(BlockingAssign* ba) {
  ba->accept_ctrl(this);
  ba->accept_assign(this);
  return ba;
}

Statement* Rewriter::rewrite(NonblockingAssign* na) {
  na->accept_ctrl(this);
  na->accept_lhs(this);
  na->accept_rhs(this);
  return na;
}

Statement* Rewriter::rewrite(CaseStatement* cs) {
  cs->accept_cond(this);
  cs->accept_items(this);
  return cs;
}

Statement* Rewriter::rewrite(ConditionalStatement* cs) {
  cs->accept_if(this);
  cs->accept_then(this);
  cs->accept_else(this);
  return cs;
}

Statement* Rewriter::rewrite(ForStatement* fs) {
  fs->accept_init(this);
  fs->accept_cond(this);
  fs->accept_update(this);
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
  pb->accept_decls(this);
  pb->accept_stmts(this);
  return pb;
}

Statement* Rewriter::rewrite(SeqBlock* sb) {
  sb->accept_id(this);
  sb->accept_decls(this);
  sb->accept_stmts(this);
  return sb;
}

Statement* Rewriter::rewrite(TimingControlStatement* tcs) {
  tcs->accept_ctrl(this);
  tcs->accept_stmt(this);
  return tcs;
}

Statement* Rewriter::rewrite(FflushStatement* fs) {
  fs->accept_fd(this);
  return fs;
}

Statement* Rewriter::rewrite(FinishStatement* fs) {
  fs->accept_arg(this);
  return fs;
}

Statement* Rewriter::rewrite(FseekStatement* fs) {
  fs->accept_fd(this);
  fs->accept_offset(this);
  fs->accept_op(this);
  return fs;
}

Statement* Rewriter::rewrite(GetStatement* gs) {
  gs->accept_fd(this);
  gs->accept_fmt(this);
  gs->accept_var(this);
  return gs;
}

Statement* Rewriter::rewrite(PutStatement* ps) {
  ps->accept_fd(this);
  ps->accept_fmt(this);
  ps->accept_expr(this);
  return ps;
}

Statement* Rewriter::rewrite(RestartStatement* rs) {
  rs->accept_arg(this);
  return rs;
}

Statement* Rewriter::rewrite(RetargetStatement* rs) {
  rs->accept_arg(this);
  return rs;
}

Statement* Rewriter::rewrite(SaveStatement* ss) {
  ss->accept_arg(this);
  return ss;
}

Statement* Rewriter::rewrite(WhileStatement* ws) {
  ws->accept_cond(this);
  ws->accept_stmt(this);
  return ws;
}

TimingControl* Rewriter::rewrite(EventControl* ec) {
  ec->accept_events(this);
  return ec;
}

VariableAssign* Rewriter::rewrite(VariableAssign* va) {
  va->accept_lhs(this);
  va->accept_rhs(this);
  return va;
}

} // namespace cascade
