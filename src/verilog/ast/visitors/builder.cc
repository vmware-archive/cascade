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

#include "src/verilog/ast/visitors/builder.h"

#include "src/verilog/ast/ast.h"

namespace cascade {

ArgAssign* Builder::build(const ArgAssign* aa) {
  return new ArgAssign(
    aa->get_exp()->accept(this),
    aa->get_imp()->accept(this)
  );
}

Attributes* Builder::build(const Attributes* a) {
  auto specs = new Many<AttrSpec>();
  for (auto s : *a->get_as()) {
    if (auto ss = s->accept(this)) {
      specs->push_back(ss);
    }
  }
  return new Attributes(
    specs
  );
}

AttrSpec* Builder::build(const AttrSpec* as) {
  return new AttrSpec(
    as->get_lhs()->accept(this),
    as->get_rhs()->accept(this)
  );
}

CaseGenerateItem* Builder::build(const CaseGenerateItem* cgi) {
  auto exprs = new Many<Expression>();
  for (auto e : *cgi->get_exprs()) {
    if (auto ee = e->accept(this)) {
      exprs->push_back(ee);
    }
  }
  return new CaseGenerateItem(
    exprs,
    cgi->get_block()->accept(this)
  );
}

CaseItem* Builder::build(const CaseItem* ci) {
  auto exprs = new Many<Expression>();
  for (auto e : *ci->get_exprs()) {
    if (auto ee = e->accept(this)) {
      exprs->push_back(ee);
    }
  }
  return new CaseItem(
    exprs,
    ci->get_stmt()->accept(this)
  );
}

Event* Builder::build(const Event* e) {
  return new Event(
    e->get_type(),
    e->get_expr()->accept(this)
  );
}

Expression* Builder::build(const BinaryExpression* be) {
  return new BinaryExpression(
    be->get_lhs()->accept(this),
    be->get_op(),
    be->get_rhs()->accept(this)
  );
}

Expression* Builder::build(const ConditionalExpression* ce) {
  return new ConditionalExpression(
    ce->get_cond()->accept(this),
    ce->get_lhs()->accept(this),
    ce->get_rhs()->accept(this)
  );
}

Expression* Builder::build(const NestedExpression* ne) {
  return new NestedExpression(
    ne->get_expr()->accept(this)
  );
}

Expression* Builder::build(const Concatenation* c) {
  auto exprs = new Many<Expression>();
  for (auto e : *c->get_exprs()) {
    if (auto ee = e->accept(this)) {
      exprs->push_back(ee);
    }
  }
  return new Concatenation(
    exprs
  );
}

Expression* Builder::build(const Identifier* id) {
  auto ids = new Many<Id>();
  for (auto i : *id->get_ids()) {
    if (auto ii = i->accept(this)) {
      ids->push_back(ii);
    }
  }
  return new Identifier(
    ids,
    id->get_dim()->accept(this)
  );
}

Expression* Builder::build(const MultipleConcatenation* mc) {
  return new MultipleConcatenation(
    mc->get_expr()->accept(this),
    mc->get_concat()->accept(this)
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
    re->get_upper()->accept(this),
    re->get_type(),
    re->get_lower()->accept(this)
  );
}

Expression* Builder::build(const UnaryExpression* ue) {
  return new UnaryExpression(
    ue->get_op(),
    ue->get_lhs()->accept(this)
  );
}

GenerateBlock* Builder::build(const GenerateBlock* gb) {
  auto items = new Many<ModuleItem>();
  for (auto i : *gb->get_items()) {
    if (auto ii = i->accept(this)) {
      items->push_back(ii);
    }
  }
  return new GenerateBlock(
    gb->get_id()->accept(this),
    gb->get_scope(),
    items
  );
}

Id* Builder::build(const Id* i) {
  return new Id(
    i->get_sid(),
    i->get_isel()->accept(this)
  );
}

IfGenerateClause* Builder::build(const IfGenerateClause* igc) {
  return new IfGenerateClause(
    igc->get_if()->accept(this),
    igc->get_then()->accept(this)
  );
}

ModuleDeclaration* Builder::build(const ModuleDeclaration* md) {
  auto ports = new Many<ArgAssign>();
  for (auto p : *md->get_ports()) {
    if (auto pp = p->accept(this)) {
      ports->push_back(pp);
    }
  }
  auto items = new Many<ModuleItem>();
  for (auto i : *md->get_items()) {
    if (auto ii = i->accept(this)) {
      items->push_back(ii);
    }
  }
  return new ModuleDeclaration(
    md->get_attrs()->accept(this),
    md->get_id()->accept(this),
    ports,
    items
  );
}

ModuleItem* Builder::build(const AlwaysConstruct* ac) {
  return new AlwaysConstruct(
    ac->get_stmt()->accept(this)
  );
}

ModuleItem* Builder::build(const IfGenerateConstruct* igc) {
  return new IfGenerateConstruct(
    igc->get_attrs()->accept(this),
    igc->get_clauses()->accept(this),
    igc->get_else()->accept(this)
  );
}

ModuleItem* Builder::build(const CaseGenerateConstruct* cgc) {
  auto items = new Many<CaseGenerateItem>();
  for (auto i : *cgc->get_items()) {
    if (auto ii = i->accept(this)) {
      items->push_back(ii);
    }
  }
  return new CaseGenerateConstruct(
    cgc->get_cond()->accept(this),
    items
  );
}

ModuleItem* Builder::build(const LoopGenerateConstruct* lgc) {
  return new LoopGenerateConstruct(
    lgc->get_init()->accept(this),
    lgc->get_cond()->accept(this),
    lgc->get_update()->accept(this),
    lgc->get_block()->accept(this)
  );
}

ModuleItem* Builder::build(const InitialConstruct* ic) {
  return new InitialConstruct(
    ic->get_attrs()->accept(this),
    ic->get_stmt()->accept(this)
  );
}

ModuleItem* Builder::build(const ContinuousAssign* ca) {
  return new ContinuousAssign(
    ca->get_ctrl()->accept(this),
    ca->get_assign()->accept(this)
  );
}

ModuleItem* Builder::build(const GenvarDeclaration* gd) {
  return new GenvarDeclaration(
    gd->get_attrs()->accept(this),
    gd->get_id()->accept(this)
  );
}

ModuleItem* Builder::build(const IntegerDeclaration* id) {
  return new IntegerDeclaration(
    id->get_attrs()->accept(this),
    id->get_id()->accept(this),
    id->get_val()->accept(this)
  );
}

ModuleItem* Builder::build(const LocalparamDeclaration* ld) {
  return new LocalparamDeclaration(
    ld->get_attrs()->accept(this),
    ld->get_dim()->accept(this),
    ld->get_id()->accept(this),
    ld->get_val()->accept(this)
  );
}

ModuleItem* Builder::build(const NetDeclaration* nd) {
  return new NetDeclaration(
    nd->get_attrs()->accept(this),
    nd->get_type(),
    nd->get_ctrl()->accept(this),
    nd->get_id()->accept(this),
    nd->get_dim()->accept(this)
  );
}

ModuleItem* Builder::build(const ParameterDeclaration* pd) {
  return new ParameterDeclaration(
    pd->get_attrs()->accept(this),
    pd->get_dim()->accept(this),
    pd->get_id()->accept(this),
    pd->get_val()->accept(this)
  );
}

ModuleItem* Builder::build(const RegDeclaration* rd) {
  return new RegDeclaration(
    rd->get_attrs()->accept(this),
    rd->get_id()->accept(this),
    rd->get_dim()->accept(this),
    rd->get_val()->accept(this)
  );
}

ModuleItem* Builder::build(const GenerateRegion* gr) {
  auto items = new Many<ModuleItem>();
  for (auto i : *gr->get_items()) {
    if (auto ii = i->accept(this)) {
      items->push_back(ii);
    }
  }
  return new GenerateRegion(
    items
  );
}

ModuleItem* Builder::build(const ModuleInstantiation* mi) {
  auto params = new Many<ArgAssign>();
  for (auto p : *mi->get_params()) {
    if (auto pp = p->accept(this)) {
      params->push_back(pp);
    }
  }
  auto ports = new Many<ArgAssign>();
  for (auto p : *mi->get_ports()) {
    if (auto pp = p->accept(this)) {
      ports->push_back(pp);
    }
  }
  return new ModuleInstantiation(
    mi->get_attrs()->accept(this),
    mi->get_mid()->accept(this),
    mi->get_iid()->accept(this),
    params,
    ports
  );
}

ModuleItem* Builder::build(const PortDeclaration* pd) {
  return new PortDeclaration(
    pd->get_attrs(),
    pd->get_type(),
    pd->get_decl()->accept(this)
  ); 
}

Statement* Builder::build(const BlockingAssign* ba) {
  return new BlockingAssign(
    ba->get_ctrl()->accept(this),
    ba->get_assign()->accept(this)
  );
}

Statement* Builder::build(const NonblockingAssign* na) {
  return new NonblockingAssign(
    na->get_ctrl()->accept(this),
    na->get_assign()->accept(this)
  );
}

Statement* Builder::build(const CaseStatement* cs) {
  auto items = new Many<CaseItem>();
  for (auto i : *cs->get_items()) {
    if (auto ii = i->accept(this)) {
      items->push_back(ii);
    }
  }
  return new CaseStatement(
    cs->get_type(),
    cs->get_cond()->accept(this),
    items
  );
}

Statement* Builder::build(const ConditionalStatement* cs) {
  return new ConditionalStatement(
    cs->get_if()->accept(this),
    cs->get_then()->accept(this),
    cs->get_else()->accept(this)
  );
}

Statement* Builder::build(const ForStatement* fs) {
  return new ForStatement(
    fs->get_init()->accept(this),
    fs->get_cond()->accept(this),
    fs->get_update()->accept(this),
    fs->get_stmt()->accept(this)
  );
}

Statement* Builder::build(const ForeverStatement* fs) {
  return new ForeverStatement(
    fs->get_stmt()->accept(this)
  );
}

Statement* Builder::build(const RepeatStatement* rs) {
  return new RepeatStatement(
    rs->get_cond()->accept(this),
    rs->get_stmt()->accept(this)
  );
}

Statement* Builder::build(const ParBlock* pb) {
  auto decls = new Many<Declaration>();
  for (auto d : *pb->get_decls()) {
    if (auto dd = d->accept(this)) {
      decls->push_back(dd);
    }
  }
  auto stmts = new Many<Statement>();
  for (auto s : *pb->get_stmts()) {
    if (auto ss = s->accept(this)) {
      stmts->push_back(ss);
    }
  }
  return new ParBlock(
    pb->get_id()->accept(this),
    decls,
    stmts
  );
}

Statement* Builder::build(const SeqBlock* sb) {
  auto decls = new Many<Declaration>();
  for (auto d : *sb->get_decls()) {
    if (auto dd = d->accept(this)) {
      decls->push_back(dd);
    }
  }
  auto stmts = new Many<Statement>();
  for (auto s : *sb->get_stmts()) {
    if (auto ss = s->accept(this)) {
      stmts->push_back(ss);
    }
  }
  return new SeqBlock(
    sb->get_id()->accept(this),
    decls,
    stmts
  );
}

Statement* Builder::build(const TimingControlStatement* tcs) {
  return new TimingControlStatement(
    tcs->get_ctrl()->accept(this),
    tcs->get_stmt()->accept(this)
  );
}

Statement* Builder::build(const DisplayStatement* ds) {
  auto args = new Many<Expression>();
  for (auto a : *ds->get_args()) {
    if (auto aa = a->accept(this)) {
      args->push_back(aa);
    }
  }
  return new DisplayStatement(
    args
  );
}

Statement* Builder::build(const FinishStatement* fs) {
  return new FinishStatement(
    fs->get_arg()->accept(this)
  );
}

Statement* Builder::build(const WriteStatement* ws) {
  auto args = new Many<Expression>();
  for (auto a : *ws->get_args()) {
    if (auto aa = a->accept(this)) {
      args->push_back(aa);
    }
  }
  return new WriteStatement(
    args
  );
}

Statement* Builder::build(const WaitStatement* ws) {
  return new WaitStatement(
    ws->get_cond()->accept(this),
    ws->get_stmt()->accept(this)
  );
}

Statement* Builder::build(const WhileStatement* ws) {
  return new WhileStatement(
    ws->get_cond()->accept(this),
    ws->get_stmt()->accept(this)
  ); 
}

TimingControl* Builder::build(const DelayControl* dc) {
  return new DelayControl(
    dc->get_delay()->accept(this)
  ); 
}

TimingControl* Builder::build(const EventControl* ec) {
  auto events = new Many<Event>();
  for (auto e : *ec->get_events()) {
    if (auto ee = e->accept(this)) {
      events->push_back(ee);
    }
  }
  return new EventControl(
    events
  );
}

VariableAssign* Builder::build(const VariableAssign* va) {
  return new VariableAssign(
    va->get_lhs()->accept(this),
    va->get_rhs()->accept(this)
  );
}

} // namespace cascade
