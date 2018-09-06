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
  aa->conditional_replace_exp(aa->get_exp()->accept(this));
  aa->conditional_replace_imp(aa->get_imp()->accept(this));
  return aa;
}

Attributes* Rewriter::rewrite(Attributes* a) {
  a->get_as()->accept(this);
  return a;
}

AttrSpec* Rewriter::rewrite(AttrSpec* as) {
  as->conditional_replace_lhs(as->get_lhs()->accept(this));
  as->conditional_replace_rhs(as->get_rhs()->accept(this));
  return as;
}

CaseGenerateItem* Rewriter::rewrite(CaseGenerateItem* cgi) {
  cgi->get_exprs()->accept(this);
  cgi->get_block()->accept(this);
  return cgi;
}

CaseItem* Rewriter::rewrite(CaseItem* ci) {
  ci->get_exprs()->accept(this);
  ci->conditional_replace_stmt(ci->get_stmt()->accept(this));
  return ci;
}

Event* Rewriter::rewrite(Event* e) {
  e->conditional_replace_expr(e->get_expr()->accept(this));
  return e;
}

Expression* Rewriter::rewrite(BinaryExpression* be) {
  be->conditional_replace_lhs(be->get_lhs()->accept(this));
  be->conditional_replace_rhs(be->get_rhs()->accept(this));
  return be;
}

Expression* Rewriter::rewrite(ConditionalExpression* ce) {
  ce->conditional_replace_cond(ce->get_cond()->accept(this));
  ce->conditional_replace_lhs(ce->get_lhs()->accept(this));
  ce->conditional_replace_rhs(ce->get_rhs()->accept(this));
  return ce;
}

Expression* Rewriter::rewrite(NestedExpression* ne) {
  ne->conditional_replace_expr(ne->get_expr()->accept(this));
  return ne;
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
  mc->conditional_replace_expr(mc->get_expr()->accept(this));
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
  re->conditional_replace_upper(re->get_upper()->accept(this));
  re->conditional_replace_lower(re->get_lower()->accept(this));
  return re;
}

Expression* Rewriter::rewrite(UnaryExpression* ue) {
  ue->conditional_replace_lhs(ue->get_lhs()->accept(this));
  return ue;
}

GenerateBlock* Rewriter::rewrite(GenerateBlock* gb) {
  gb->get_id()->accept(this);
  gb->get_items()->accept(this);
  return gb;
}

Id* Rewriter::rewrite(Id* i) {
  i->get_isel()->accept(this);
  return i;
}

ModuleDeclaration* Rewriter::rewrite(ModuleDeclaration* md) {
  md->get_attrs()->accept(this);
  md->get_id()->accept(this);
  md->get_ports()->accept(this);
  md->get_items()->accept(this);
  return md;
}

ModuleItem* Rewriter::rewrite(AlwaysConstruct* ac) {
  ac->conditional_replace_stmt(ac->get_stmt()->accept(this));
  return ac;
}

ModuleItem* Rewriter::rewrite(IfGenerateConstruct* igc) {
  igc->get_attrs()->accept(this);
  igc->conditional_replace_if(igc->get_if()->accept(this));
  igc->get_then()->accept(this);
  igc->get_else()->accept(this);
  return igc;
}

ModuleItem* Rewriter::rewrite(CaseGenerateConstruct* cgc) {
  cgc->conditional_replace_cond(cgc->get_cond()->accept(this));
  cgc->get_items()->accept(this);
  return cgc;
}

ModuleItem* Rewriter::rewrite(LoopGenerateConstruct* lgc) {
  lgc->get_init()->accept(this);
  lgc->conditional_replace_cond(lgc->get_cond()->accept(this));
  lgc->get_update()->accept(this);
  lgc->get_block()->accept(this);
  return lgc;
}

ModuleItem* Rewriter::rewrite(InitialConstruct* ic) {
  ic->get_attrs()->accept(this);
  ic->conditional_replace_stmt(ic->get_stmt()->accept(this));
  return ic;
}

ModuleItem* Rewriter::rewrite(ContinuousAssign* ca) {
  ca->get_ctrl()->accept(this);
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
  id->conditional_replace_val(id->get_val()->accept(this));
  return id;
}

ModuleItem* Rewriter::rewrite(LocalparamDeclaration* ld) {
  ld->get_attrs()->accept(this);
  ld->get_dim()->accept(this);
  ld->get_id()->accept(this);
  ld->conditional_replace_val(ld->get_val()->accept(this));
  return ld;
}

ModuleItem* Rewriter::rewrite(NetDeclaration* nd) {
  nd->get_attrs()->accept(this);
  nd->get_ctrl()->accept(this);
  nd->get_id()->accept(this);
  nd->get_dim()->accept(this);
  return nd;
}

ModuleItem* Rewriter::rewrite(ParameterDeclaration* pd) {
  pd->get_attrs()->accept(this);
  pd->get_dim()->accept(this);
  pd->get_id()->accept(this);
  pd->conditional_replace_val(pd->get_val()->accept(this));
  return pd;
}

ModuleItem* Rewriter::rewrite(RegDeclaration* rd) {
  rd->get_attrs()->accept(this);
  rd->get_id()->accept(this);
  rd->get_dim()->accept(this);
  rd->conditional_replace_val(rd->get_val()->accept(this));
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
  ba->get_ctrl()->accept(this);
  ba->get_assign()->accept(this);
  return ba;
}

Statement* Rewriter::rewrite(NonblockingAssign* na) {
  na->get_ctrl()->accept(this);
  na->get_assign()->accept(this);
  return na;
}

Statement* Rewriter::rewrite(CaseStatement* cs) {
  cs->conditional_replace_cond(cs->get_cond()->accept(this));
  cs->get_items()->accept(this);
  return cs;
}

Statement* Rewriter::rewrite(ConditionalStatement* cs) {
  cs->conditional_replace_if(cs->get_if()->accept(this));
  cs->conditional_replace_then(cs->get_then()->accept(this));
  cs->conditional_replace_else(cs->get_else()->accept(this));
  return cs;
}

Statement* Rewriter::rewrite(ForStatement* fs) {
  fs->get_init()->accept(this);
  fs->conditional_replace_cond(fs->get_cond()->accept(this));
  fs->get_update()->accept(this);
  fs->conditional_replace_stmt(fs->get_stmt()->accept(this));
  return fs;
}

Statement* Rewriter::rewrite(ForeverStatement* fs) {
  fs->conditional_replace_stmt(fs->get_stmt()->accept(this));
  return fs;
}

Statement* Rewriter::rewrite(RepeatStatement* rs) {
  rs->conditional_replace_cond(rs->get_cond()->accept(this));
  rs->conditional_replace_stmt(rs->get_stmt()->accept(this));
  return rs;
}

Statement* Rewriter::rewrite(ParBlock* pb) {
  pb->get_id()->accept(this);
  pb->get_decls()->accept(this);
  pb->get_stmts()->accept(this);
  return pb;
}

Statement* Rewriter::rewrite(SeqBlock* sb) {
  sb->get_id()->accept(this);
  sb->get_decls()->accept(this);
  sb->get_stmts()->accept(this);
  return sb;
}

Statement* Rewriter::rewrite(TimingControlStatement* tcs) {
  tcs->get_ctrl()->accept(this);
  tcs->conditional_replace_stmt(tcs->get_stmt()->accept(this));
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
  ws->conditional_replace_cond(ws->get_cond()->accept(this));
  ws->conditional_replace_stmt(ws->get_stmt()->accept(this));
  return ws;
}

Statement* Rewriter::rewrite(WhileStatement* ws) {
  ws->conditional_replace_cond(ws->get_cond()->accept(this));
  ws->conditional_replace_stmt(ws->get_stmt()->accept(this));
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
  va->conditional_replace_lhs(va->get_lhs()->accept(this));
  va->conditional_replace_rhs(va->get_rhs()->accept(this));
  return va;
}

} // namespace cascade
