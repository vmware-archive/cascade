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

#include "verilog/ast/visitors/builder.h"

#include "verilog/ast/ast.h"

namespace cascade {

ArgAssign* Builder::build(const ArgAssign* aa) {
  return new ArgAssign(
    aa->accept_exp(this),
    aa->accept_imp(this)
  );
}

Attributes* Builder::build(const Attributes* a) {
  auto* res = new Attributes();
  a->accept_as(this, res->back_inserter_as());
  return res;
}

AttrSpec* Builder::build(const AttrSpec* as) {
  return new AttrSpec(
    as->accept_lhs(this),
    as->accept_rhs(this)
  );
}

CaseGenerateItem* Builder::build(const CaseGenerateItem* cgi) {
  auto* res = new CaseGenerateItem();
  res->replace_block(cgi->accept_block(this));
  cgi->accept_exprs(this, res->back_inserter_exprs());
  return res;
}

CaseItem* Builder::build(const CaseItem* ci) {
  auto* res = new CaseItem(
    ci->accept_stmt(this)
  );
  ci->accept_exprs(this, res->back_inserter_exprs());
  return res;
}

Event* Builder::build(const Event* e) {
  return new Event(
    e->get_type(),
    e->accept_expr(this)
  );
}

Expression* Builder::build(const BinaryExpression* be) {
  return new BinaryExpression(
    be->accept_lhs(this),
    be->get_op(),
    be->accept_rhs(this)
  );
}

Expression* Builder::build(const ConditionalExpression* ce) {
  return new ConditionalExpression(
    ce->accept_cond(this),
    ce->accept_lhs(this),
    ce->accept_rhs(this)
  );
}

Expression* Builder::build(const FeofExpression* fe) {
  return new FeofExpression(
    fe->accept_arg(this)
  );
}

Expression* Builder::build(const FopenExpression* fe) {
  return new FopenExpression(
    fe->accept_arg(this)
  );
}

Expression* Builder::build(const Concatenation* c) {
  auto* res = new Concatenation();
  c->accept_exprs(this, res->back_inserter_exprs());
  return res;
}

Expression* Builder::build(const Identifier* id) {
  auto* res = new Identifier();
  id->accept_ids(this, res->back_inserter_ids());
  id->accept_dim(this, res->back_inserter_dim());
  return res;
}

Expression* Builder::build(const MultipleConcatenation* mc) {
  return new MultipleConcatenation(
    mc->accept_expr(this),
    mc->accept_concat(this)
  );
}

Expression* Builder::build(const Number* n) {
  return n->clone();
}

Expression* Builder::build(const String* s) {
  return s->clone();
}

Expression* Builder::build(const RangeExpression* re) {
  return new RangeExpression(
    re->accept_upper(this),
    re->get_type(),
    re->accept_lower(this)
  );
}

Expression* Builder::build(const UnaryExpression* ue) {
  return new UnaryExpression(
    ue->get_op(),
    ue->accept_lhs(this)
  );
}

GenerateBlock* Builder::build(const GenerateBlock* gb) {
  auto* res = new GenerateBlock(
    gb->get_scope()
  );
  res->replace_id(gb->accept_id(this));
  gb->accept_items(this, res->back_inserter_items());
  return res;
}

Id* Builder::build(const Id* i) {
  return new Id(
    i->get_sid(),
    i->accept_isel(this)
  );
}

IfGenerateClause* Builder::build(const IfGenerateClause* igc) {
  return new IfGenerateClause(
    igc->accept_if(this),
    igc->accept_then(this)
  );
}

ModuleDeclaration* Builder::build(const ModuleDeclaration* md) {
  auto* res = new ModuleDeclaration(
    md->accept_attrs(this),
    md->accept_id(this)
  );
  md->accept_ports(this, res->back_inserter_ports());
  md->accept_items(this, res->back_inserter_items());
  return res;
}

ModuleItem* Builder::build(const AlwaysConstruct* ac) {
  return new AlwaysConstruct(
    ac->accept_stmt(this)
  );
}

ModuleItem* Builder::build(const IfGenerateConstruct* igc) {
  auto* res = new IfGenerateConstruct(
    igc->accept_attrs(this)
  );
  igc->accept_clauses(this, res->back_inserter_clauses());
  res->replace_else(igc->accept_else(this));
  return res;
}

ModuleItem* Builder::build(const CaseGenerateConstruct* cgc) {
  auto* res = new CaseGenerateConstruct(
    cgc->accept_cond(this)
  );
  cgc->accept_items(this, res->back_inserter_items());
  return res;
}

ModuleItem* Builder::build(const LoopGenerateConstruct* lgc) {
  return new LoopGenerateConstruct(
    lgc->accept_init(this),
    lgc->accept_cond(this),
    lgc->accept_update(this),
    lgc->accept_block(this)
  );
}

ModuleItem* Builder::build(const InitialConstruct* ic) {
  return new InitialConstruct(
    ic->accept_attrs(this),
    ic->accept_stmt(this)
  );
}

ModuleItem* Builder::build(const ContinuousAssign* ca) {
  return new ContinuousAssign(
    ca->accept_ctrl(this),
    ca->accept_assign(this)
  );
}

ModuleItem* Builder::build(const GenvarDeclaration* gd) {
  return new GenvarDeclaration(
    gd->accept_attrs(this),
    gd->accept_id(this)
  );
}

ModuleItem* Builder::build(const IntegerDeclaration* id) {
  return new IntegerDeclaration(
    id->accept_attrs(this),
    id->accept_id(this),
    id->accept_val(this)
  );
}

ModuleItem* Builder::build(const LocalparamDeclaration* ld) {
  return new LocalparamDeclaration(
    ld->accept_attrs(this),
    ld->get_signed(),
    ld->accept_dim(this),
    ld->accept_id(this),
    ld->accept_val(this)
  );
}

ModuleItem* Builder::build(const NetDeclaration* nd) {
  return new NetDeclaration(
    nd->accept_attrs(this),
    nd->get_type(),
    nd->accept_ctrl(this),
    nd->accept_id(this),
    nd->get_signed(),
    nd->accept_dim(this)
  );
}

ModuleItem* Builder::build(const ParameterDeclaration* pd) {
  return new ParameterDeclaration(
    pd->accept_attrs(this),
    pd->get_signed(),
    pd->accept_dim(this),
    pd->accept_id(this),
    pd->accept_val(this)
  );
}

ModuleItem* Builder::build(const RegDeclaration* rd) {
  return new RegDeclaration(
    rd->accept_attrs(this),
    rd->accept_id(this),
    rd->get_signed(),
    rd->accept_dim(this),
    rd->accept_val(this)
  );
}

ModuleItem* Builder::build(const GenerateRegion* gr) {
  auto* res = new GenerateRegion();
  gr->accept_items(this, res->back_inserter_items());
  return res;
}

ModuleItem* Builder::build(const ModuleInstantiation* mi) {
  auto* res = new ModuleInstantiation(
    mi->accept_attrs(this),
    mi->accept_mid(this),
    mi->accept_iid(this)
  );
  res->replace_range(mi->accept_range(this));
  mi->accept_params(this, res->back_inserter_params());
  mi->accept_ports(this, res->back_inserter_ports());
  return res;
}

ModuleItem* Builder::build(const PortDeclaration* pd) {
  return new PortDeclaration(
    pd->accept_attrs(this),
    pd->get_type(),
    pd->accept_decl(this)
  ); 
}

Statement* Builder::build(const BlockingAssign* ba) {
  return new BlockingAssign(
    ba->accept_ctrl(this),
    ba->accept_assign(this)
  );
}

Statement* Builder::build(const NonblockingAssign* na) {
  return new NonblockingAssign(
    na->accept_ctrl(this),
    na->accept_assign(this)
  );
}

Statement* Builder::build(const CaseStatement* cs) {
  auto* res = new CaseStatement(
    cs->get_type(),
    cs->accept_cond(this)
  );
  cs->accept_items(this, res->back_inserter_items());
  return res;
}

Statement* Builder::build(const ConditionalStatement* cs) {
  return new ConditionalStatement(
    cs->accept_if(this),
    cs->accept_then(this),
    cs->accept_else(this)
  );
}

Statement* Builder::build(const ForStatement* fs) {
  return new ForStatement(
    fs->accept_init(this),
    fs->accept_cond(this),
    fs->accept_update(this),
    fs->accept_stmt(this)
  );
}

Statement* Builder::build(const ForeverStatement* fs) {
  return new ForeverStatement(
    fs->accept_stmt(this)
  );
}

Statement* Builder::build(const RepeatStatement* rs) {
  return new RepeatStatement(
    rs->accept_cond(this),
    rs->accept_stmt(this)
  );
}

Statement* Builder::build(const ParBlock* pb) {
  auto* res = new ParBlock();
  res->replace_id(pb->accept_id(this));
  pb->accept_decls(this, res->back_inserter_decls());
  pb->accept_stmts(this, res->back_inserter_stmts());
  return res;
}

Statement* Builder::build(const SeqBlock* sb) {
  auto* res = new SeqBlock();
  res->replace_id(sb->accept_id(this));
  sb->accept_decls(this, res->back_inserter_decls());
  sb->accept_stmts(this, res->back_inserter_stmts());
  return res;
}

Statement* Builder::build(const TimingControlStatement* tcs) {
  return new TimingControlStatement(
    tcs->accept_ctrl(this),
    tcs->accept_stmt(this)
  );
}

Statement* Builder::build(const DisplayStatement* ds) {
  auto* res = new DisplayStatement();
  ds->accept_args(this, res->back_inserter_args());
  return res;
}

Statement* Builder::build(const ErrorStatement* es) {
  auto* res = new ErrorStatement();
  es->accept_args(this, res->back_inserter_args());
  return res;
}

Statement* Builder::build(const FinishStatement* fs) {
  return new FinishStatement(
    fs->accept_arg(this)
  );
}

Statement* Builder::build(const FseekStatement* fs) {
  return new FseekStatement(
    fs->accept_id(this),
    fs->accept_pos(this)
  );
}

Statement* Builder::build(const GetStatement* gs) {
  return new GetStatement(
    gs->accept_id(this),
    gs->accept_var(this)
  );
}

Statement* Builder::build(const InfoStatement* is) {
  auto* res = new InfoStatement();
  is->accept_args(this, res->back_inserter_args());
  return res;
}

Statement* Builder::build(const PutStatement* ps) {
  return new PutStatement(
    ps->accept_id(this),
    ps->accept_var(this)
  );
}

Statement* Builder::build(const RestartStatement* rs) {
  return new RestartStatement(
    rs->accept_arg(this)
  );
}

Statement* Builder::build(const RetargetStatement* rs) {
  return new RetargetStatement(
    rs->accept_arg(this)
  );
}

Statement* Builder::build(const SaveStatement* ss) {
  return new SaveStatement(
    ss->accept_arg(this)
  );
}

Statement* Builder::build(const WarningStatement* ws) {
  auto* res = new WarningStatement();
  ws->accept_args(this, res->back_inserter_args());
  return res;
}

Statement* Builder::build(const WriteStatement* ws) {
  auto* res = new WriteStatement();
  ws->accept_args(this, res->back_inserter_args());
  return res;
}

Statement* Builder::build(const WaitStatement* ws) {
  return new WaitStatement(
    ws->accept_cond(this),
    ws->accept_stmt(this)
  );
}

Statement* Builder::build(const WhileStatement* ws) {
  return new WhileStatement(
    ws->accept_cond(this),
    ws->accept_stmt(this)
  ); 
}

TimingControl* Builder::build(const DelayControl* dc) {
  return new DelayControl(
    dc->accept_delay(this)
  ); 
}

TimingControl* Builder::build(const EventControl* ec) {
  auto* res = new EventControl();
  ec->accept_events(this, res->back_inserter_events());
  return res;
}

VariableAssign* Builder::build(const VariableAssign* va) {
  return new VariableAssign(
    va->accept_lhs(this),
    va->accept_rhs(this)
  );
}

} // namespace cascade
