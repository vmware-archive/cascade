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

#include "verilog/print/printer.h"

#include <array>
#include <cassert>
#include "verilog/ast/ast.h"

using namespace std;

namespace cascade {

Printer::Printer(ostream& os) : os_(os) { 
  os_.delim("\t");
}

void Printer::visit(const ArgAssign* aa) {
  if (aa->is_non_null_exp()) {
    *this << Color::RED << "." << Color::RESET;
    aa->accept_exp(this); 
    *this << Color::RED << "(" << Color::RESET;
  }
  aa->accept_imp(this);
  if (aa->is_non_null_exp()) {
    *this << Color::RED << ")" << Color::RESET;
  } 
}

void Printer::visit(const Attributes* as) {
  if (as->empty_as()) {
    return;
  }
  *this << Color::RED << "(*" << Color::RESET;
  int cnt = 0;
  as->accept_as(this, [this,&cnt]{if (cnt++) {*this << Color::RED << "," << Color::RESET;}}, []{});
  *this << Color::RED << "*) " << Color::RESET;
}

void Printer::visit(const AttrSpec* as) {
  as->accept_lhs(this);
  if (as->is_non_null_rhs()) {
    *this << Color::RED << " = " << Color::RESET;
    as->accept_rhs(this);
  }
}

void Printer::visit(const CaseGenerateItem* cgi) {
  if (cgi->empty_exprs()) {
    *this << Color::GREEN << "default" << Color::RESET;
  } 
  int cnt = 0;
  cgi->accept_exprs(this, [this,&cnt]{if (cnt++) {*this << Color::RED << ",\n" << Color::RESET;}}, []{});
  *this << Color::RED << ": " << Color::RESET;
  cgi->accept_block(this);
}

void Printer::visit(const CaseItem* ci) {
  if (ci->empty_exprs()) {
    *this << Color::GREEN << "default" << Color::RESET;
  } 
  int cnt = 0;
  ci->accept_exprs(this, [this,&cnt]{if (cnt++) {*this << Color::RED << ",\n" << Color::RESET;}}, []{});
  *this << Color::RED << ": " << Color::RESET;
  ci->accept_stmt(this);
}

void Printer::visit(const Event* e) {
  static array<string,3> ets_ {{"","negedge ","posedge "}};
  *this << Color::GREEN << ets_[static_cast<size_t>(e->get_type())] << Color::RESET;
  e->accept_expr(this);
}

void Printer::visit(const BinaryExpression* be) {
  static array<string,24> ops_ {{
    "+",  "-",   "*",  "/",  "%", "===", "==",  "!==", "!=", "&&",
    "||", "**",  "<",  "<=", ">", ">=",  "&",   "|",   "^",  "~^",
    "<<", "<<<", ">>", ">>>"
  }};
  *this << Color::RED << "(" << Color::RESET;
  be->accept_lhs(this);
  *this << " ";
  *this << Color::RED << ops_[static_cast<size_t>(be->get_op())] << Color::RESET;
  *this << " ";
  be->accept_rhs(this);
  *this << Color::RED << ")" << Color::RESET;
}

void Printer::visit(const ConditionalExpression* ce) {
  *this << Color::RED << "(" << Color::RESET;
  ce->accept_cond(this);
  *this << Color::RED << " ? " << Color::RESET;
  ce->accept_lhs(this);
  *this << Color::RED << " : " << Color::RESET;
  ce->accept_rhs(this);
  *this << Color::RED << ")" << Color::RESET;
}

void Printer::visit(const FeofExpression* fe) {
  *this << Color::YELLOW << "$feof" << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  fe->accept_fd(this);
  *this << Color::RED << ")" << Color::RESET;
}

void Printer::visit(const FopenExpression* fe) {
  *this << Color::YELLOW << "$fopen" << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  fe->accept_path(this);
  *this << Color::RED << ")" << Color::RESET;
}

void Printer::visit(const Concatenation* c) {
  *this << Color::RED << "{" << Color::RESET;
  int cnt = 0;
  c->accept_exprs(this, [this,&cnt]{if (cnt++) {*this << Color::RED << "," << Color::RESET;}}, []{});
  *this << Color::RED << "}" << Color::RESET;
}

void Printer::visit(const Identifier* id) {
  int cnt = 0;
  id->accept_ids(this, [this,&cnt]{if (cnt++) {*this << ".";}}, []{});
  for (auto i = id->begin_dim(), ie = id->end_dim(); i != ie; ++i) {
    *this << Color::RED << "[" << Color::RESET;
    (*i)->accept(this);
    *this << Color::RED << "]" << Color::RESET;
  }
}

void Printer::visit(const MultipleConcatenation* mc) {
  *this << Color::RED << "{" << Color::RESET;
  mc->accept_expr(this);
  *this << " ";
  mc->accept_concat(this);
  *this << Color::RED << "}" << Color::RESET;
}

void Printer::visit(const Number* n) {
  *this << Color::BLUE;
  switch (n->get_format()) {
    case Number::Format::UNBASED:
      n->get_val().write(os_, 10);
      break;
    case Number::Format::REAL:
      n->get_val().write(os_, 1);
      break;
    case Number::Format::DEC:
      *this << n->get_val().size() << "'" << (n->get_val().is_signed() ? "sd" : "d");
      n->get_val().write(os_, 10);
      break;
    case Number::Format::BIN:
      *this << n->get_val().size() << "'" << (n->get_val().is_signed() ? "sb" : "b");
      n->get_val().write(os_, 2);
      break;
    case Number::Format::OCT:
      *this << n->get_val().size() << "'" << (n->get_val().is_signed() ? "so" : "o");
      n->get_val().write(os_, 8);
      break;
    case Number::Format::HEX:
      *this << n->get_val().size() << "'" << (n->get_val().is_signed() ? "sh" : "h");
      n->get_val().write(os_, 16);
      break;
    default:
      assert(false);
      break;
  } 
  *this << Color::RESET;
}

void Printer::visit(const String* s) {
  *this << Color::BLUE << "\"";
  for (auto c : s->get_readable_val()) {
    if (c == '\n') {
      os_ << "\\n";
    } else {
      os_ << c;
    }
  }
  *this << "\"" << Color::RESET;
}

void Printer::visit(const RangeExpression* re) {
  static array<string,3> sts_ {{":","+:","-:"}};
  re->accept_upper(this);
  *this << Color::RED << sts_[static_cast<size_t>(re->get_type())] << Color::RESET;
  re->accept_lower(this);
}

void Printer::visit(const UnaryExpression* ue) {
  static array<string,10> ops_ {{"+",  "-",  "!",  "~",  "&", "~&", "|",  "~|", "^", "~^"}};
  *this << Color::RED << ops_[static_cast<size_t>(ue->get_op())] << Color::RESET;
  ue->accept_lhs(this);
}

void Printer::visit(const GenerateBlock* gb) {
  const auto surround = gb->get_scope() || (gb->size_items() > 1) || gb->is_non_null_id();
  if (surround) {
    *this << Color::GREEN << "begin" << Color::RESET;
  }
  if (gb->is_non_null_id()) {
    *this << Color::RED << " : " << Color::RESET;
    gb->accept_id(this);
  }
  os_.tab();
  gb->accept_items(this, [this]{*this << "\n";}, []{});
  os_.untab();
  if (surround) {
    *this << Color::GREEN << "\nend " << Color::RESET;
  }
}

void Printer::visit(const Id* id) {
  *this << id->get_readable_sid();
  if (id->is_non_null_isel()) {
    *this << Color::RED << "[" << Color::RESET;
    id->accept_isel(this);
    *this << Color::RED << "]" << Color::RESET;
  }
}

void Printer::visit(const IfGenerateClause* igc) {
  *this << Color::GREEN << "if " << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  igc->accept_if(this);
  *this << Color::RED << ") " << Color::RESET;
  igc->accept_then(this);
}

void Printer::visit(const ModuleDeclaration* md) {
  md->accept_attrs(this);
  if (!md->get_attrs()->empty_as()) {
    *this << "\n";
  }
  *this << Color::GREEN << "module " << Color::RESET;
  md->accept_id(this);
  *this << Color::RED << "(" << Color::RESET;
  int cnt = 0;
  md->accept_ports(this, [this,&cnt]{if (cnt++) {*this << Color::RED << "," << Color::RESET;}}, []{});
  *this << Color::RED << ");" << Color::RESET << "\n";
  os_.tab();
  md->accept_items(this, []{}, [this]{*this << "\n";});
  os_.untab();
  *this << Color::GREEN << "endmodule" << Color::RESET;
}

void Printer::visit(const IfGenerateConstruct* igc) {
  igc->accept_attrs(this);
  if (!igc->get_attrs()->empty_as()) {
    *this << "\n";
  }
  auto c = igc->begin_clauses();
  (*c)->accept(this);
  ++c;
  for (auto ce = igc->end_clauses(); c != ce; ++c) {
    *this << Color::GREEN << "else " << Color::RESET;
    (*c)->accept(this);
  }
  if (igc->is_non_null_else()) {
    *this << Color::GREEN << "else " << Color::RESET;
    igc->accept_else(this);
  }
}

void Printer::visit(const AlwaysConstruct* ac) {
  *this << Color::GREEN << "always " << Color::RESET;
  ac->accept_stmt(this);
}

void Printer::visit(const CaseGenerateConstruct* cgc) {
  *this << Color::GREEN << "case" << Color::RESET;
  *this << Color::RED << " (" << Color::RESET;
  cgc->accept_cond(this);
  *this << Color::RED << ")" << Color::RESET << "\n";
  os_.tab();
  cgc->accept_items(this, []{}, [this]{*this << "\n";});
  os_.untab();
  *this << Color::GREEN << "endcase" << Color::RESET;
}

void Printer::visit(const LoopGenerateConstruct* lgc) {
  *this << Color::GREEN << "for " << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  lgc->accept_init(this);
  *this << Color::RED << "; " << Color::RESET;
  lgc->accept_cond(this); 
  *this << Color::RED << "; " << Color::RESET;
  lgc->accept_update(this);
  *this << Color::RED << ") " << Color::RESET;
  lgc->accept_block(this);
}

void Printer::visit(const InitialConstruct* ic) {
  ic->accept_attrs(this);
  if (!ic->get_attrs()->empty_as()) {
    *this << "\n";
  }
  *this << Color::GREEN << "initial " << Color::RESET;
  ic->accept_stmt(this);
}

void Printer::visit(const ContinuousAssign* ca) {
  *this << Color::GREEN << "assign " << Color::RESET;
  ca->accept_lhs(this);
  *this << Color::RED << " = " << Color::RESET;
  ca->accept_rhs(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const GenvarDeclaration* gd) {
  gd->accept_attrs(this);
  *this << Color::GREEN << "genvar " << Color::RESET;
  gd->accept_id(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const LocalparamDeclaration* ld) {
  ld->accept_attrs(this);
  *this << Color::GREEN << "localparam" << Color::RESET;
  if (ld->get_type() == Declaration::Type::REAL) {
    *this << Color::GREEN << " real" << Color::RESET;
  } else {
    if (ld->get_type() == Declaration::Type::SIGNED) {
      *this << Color::GREEN << " signed" << Color::RESET;
    }
    if (ld->is_non_null_dim()) {
      *this << Color::RED << "[" << Color::RESET;
      ld->accept_dim(this);
      *this << Color::RED << "]" << Color::RESET;
    }
  }
  *this << " ";
  ld->accept_id(this);
  *this << Color::RED << " = " << Color::RESET;
  ld->accept_val(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const NetDeclaration* nd) {
  nd->accept_attrs(this);
  *this << Color::GREEN << "wire" << Color::RESET;
  if (nd->get_type() == Declaration::Type::SIGNED) {
    *this << Color::GREEN << " signed" << Color::RESET;
  }
  if (nd->is_non_null_dim()) {
    *this << Color::RED << "[" << Color::RESET;
    nd->accept_dim(this);
    *this << Color::RED << "]" << Color::RESET;
  }
  *this << " ";
  nd->accept_id(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const ParameterDeclaration* pd) {
  pd->accept_attrs(this);
  *this << Color::GREEN << "parameter" << Color::RESET;
  if (pd->get_type() == Declaration::Type::REAL) {
    *this << Color::GREEN << " real" << Color::RESET;
  } else {
    if (pd->get_type() == Declaration::Type::SIGNED) {
      *this << Color::GREEN << " signed" << Color::RESET;
    }
    if (pd->is_non_null_dim()) {
      *this << Color::RED << "[" << Color::RESET;
      pd->accept_dim(this);
      *this << Color::RED << "]" << Color::RESET;
    }
  }
  *this << " ";
  pd->accept_id(this);
  *this << Color::RED << " = " << Color::RESET;
  pd->accept_val(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const RegDeclaration* rd) {
  rd->accept_attrs(this);
  if (rd->get_type() == Declaration::Type::REAL) {
    *this << Color::GREEN << "real" << Color::RESET;
  } else {
    *this << Color::GREEN << "reg" << Color::RESET;
    if (rd->get_type() == Declaration::Type::SIGNED) {
      *this << Color::GREEN << " signed" << Color::RESET;
    }
    if (rd->is_non_null_dim()) {
      *this << Color::RED << "[" << Color::RESET;
      rd->accept_dim(this);
      *this << Color::RED << "]" << Color::RESET;
    }
  }
  *this << " ";
  rd->accept_id(this);
  if (rd->is_non_null_val()) {
    *this << Color::RED << " = " << Color::RESET;
    rd->accept_val(this);
  }
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const GenerateRegion* gr) {
  *this << Color::GREEN << "generate" << Color::RESET << "\n";
  os_.tab();
  gr->accept_items(this, []{}, [this]{*this << "\n";});
  os_.untab();
  *this << Color::GREEN << "endgenerate" << Color::RESET;
}

void Printer::visit(const ModuleInstantiation* mi) {
  mi->accept_attrs(this);
  if (!mi->get_attrs()->empty_as()) {
    *this << "\n";
  }
  mi->accept_mid(this);
  *this << " ";
  if (!mi->empty_params()) {
    *this << Color::RED << "#(" << Color::RESET;
    int cnt = 0;
    mi->accept_params(this, [this,&cnt]{if (cnt++) {*this << Color::RED << "," << Color::RESET;}}, []{});
    *this << Color::RED << ") " << Color::RESET;
  }
  mi->accept_iid(this);
  if (mi->is_non_null_range()) {
    *this << Color::RED << "[" << Color::RESET;
    mi->accept_range(this);
    *this << Color::RED << "]" << Color::RESET;
  }
  *this << Color::RED << "(" << Color::RESET;
  int cnt = 0;
  mi->accept_ports(this, [this,&cnt]{if (cnt++) {*this << Color::RED << "," << Color::RESET;}}, []{});
  *this << Color::RED << ");" << Color::RESET;
}

void Printer::visit(const PortDeclaration* pd) { 
  static array<string,3> pts_ {{"inout","input","output"}};
  pd->accept_attrs(this);
  *this << Color::GREEN << pts_[static_cast<size_t>(pd->get_type())] << Color::RESET;
  *this << " ";
  pd->accept_decl(this);
}

void Printer::visit(const BlockingAssign* ba) {
  ba->get_assign()->accept_lhs(this);
  *this << Color::RED << " = " << Color::RESET;
  ba->accept_ctrl(this, []{}, [this]{*this << " ";});
  ba->get_assign()->accept_rhs(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const NonblockingAssign* na) {
  na->get_assign()->accept_lhs(this);
  *this << Color::RED << " <= " << Color::RESET;
  na->accept_ctrl(this, []{}, [this]{*this << " ";});
  na->get_assign()->accept_rhs(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const CaseStatement* cs) {
  static array<string,3> cts_ {{"case","casex","casez"}};
  *this << Color::GREEN << cts_[static_cast<size_t>(cs->get_type())] << Color::RESET;
  *this << Color::RED << " (" << Color::RESET;
  cs->accept_cond(this);
  *this << Color::RED << ")" << Color::RESET << "\n";
  os_.tab();
  cs->accept_items(this, []{}, [this]{*this << "\n";});
  os_.untab();
  *this << Color::GREEN << "endcase" << Color::RESET;
}

void Printer::visit(const ConditionalStatement* cs) {
  *this << Color::GREEN << "if " << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  cs->accept_if(this);
  *this << Color::RED << ") " << Color::RESET;

  const auto tpb = cs->get_then()->is(Node::Tag::par_block);
  const auto tsb = cs->get_then()->is(Node::Tag::seq_block);
  if (!tpb && !tsb) {
    os_.tab();
    *this << "\n";
    cs->accept_then(this);
    os_.untab();
  } else {
    cs->accept_then(this);
  }

  const auto epb = cs->get_else()->is(Node::Tag::par_block);
  const auto esb = cs->get_else()->is(Node::Tag::seq_block);
  if (!epb && !esb) {
    *this << "\n";
    *this << Color::GREEN << "else " << Color::RESET;
    os_.tab();
    *this << "\n";
    cs->accept_else(this);
    os_.untab();
  } else if ((epb && !static_cast<const ParBlock*>(cs->get_else())->empty_stmts()) || 
             (esb && !static_cast<const SeqBlock*>(cs->get_else())->empty_stmts())) {
    *this << "\n";
    *this << Color::GREEN << "else " << Color::RESET;
    cs->accept_else(this);
  }
}

void Printer::visit(const ForStatement* fs) {
  *this << Color::GREEN << "for " << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  fs->accept_init(this);
  *this << Color::RED << "; " << Color::RESET;
  fs->accept_cond(this); 
  *this << Color::RED << "; " << Color::RESET;
  fs->accept_update(this);
  *this << Color::RED << ") " << Color::RESET;
  fs->accept_stmt(this);
}

void Printer::visit(const RepeatStatement* rs) {
  *this << Color::GREEN << "repeat " << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  rs->accept_cond(this); 
  *this << Color::RED << ") " << Color::RESET;
  rs->accept_stmt(this);
}

void Printer::visit(const ParBlock* pb) { 
  *this << Color::GREEN << "fork" << Color::RESET;
  if (pb->is_non_null_id()) {
    *this << Color::RED << " : " << Color::RESET;
    pb->accept_id(this);
  }
  *this << "\n";
  os_.tab();
  pb->accept_decls(this, []{}, [this]{*this << "\n";});
  pb->accept_stmts(this, []{}, [this]{*this << "\n";});
  os_.untab();
  *this << Color::GREEN << "join " << Color::RESET;
}

void Printer::visit(const SeqBlock* sb) { 
  *this << Color::GREEN << "begin" << Color::RESET;
  if (sb->is_non_null_id()) {
    *this << Color::RED << " : " << Color::RESET;
    sb->accept_id(this);
  }
  os_.tab();
  *this << "\n";
  sb->accept_decls(this, []{}, [this]{*this << "\n";});
  sb->accept_stmts(this, []{}, [this]{*this << "\n";});
  os_.untab();
  *this << Color::GREEN << "end " << Color::RESET;
}

void Printer::visit(const TimingControlStatement* ptc) {
  ptc->accept_ctrl(this);
  *this << " ";
  ptc->accept_stmt(this);
}

void Printer::visit(const FflushStatement* fs) {
  *this << Color::YELLOW << "$fflush" << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  fs->accept_fd(this);
  *this << Color::RED << ");" << Color::RESET;
}

void Printer::visit(const FinishStatement* fs) {
  *this << Color::YELLOW << "$finish" << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  fs->accept_arg(this);
  *this << Color::RED << ");" << Color::RESET;
}

void Printer::visit(const FseekStatement* fs) {
  *this << Color::YELLOW << "$fseek" << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  fs->accept_fd(this);
  *this << Color::RED << "," << Color::RESET;
  fs->accept_offset(this);
  *this << Color::RED << "," << Color::RESET;
  fs->accept_op(this);
  *this << Color::RED << ");" << Color::RESET;
}

void Printer::visit(const GetStatement* gs) {
  *this << Color::YELLOW << "$__get" << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  gs->accept_fd(this);
  *this << Color::RED << "," << Color::RESET;
  gs->accept_fmt(this);
  if (gs->is_non_null_var()) {
  *this << Color::RED << "," << Color::RESET;
  gs->accept_var(this);
  }
  *this << Color::RED << ");" << Color::RESET;
}

void Printer::visit(const PutStatement* ps) {
  *this << Color::YELLOW << "$__put" << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  ps->accept_fd(this);
  *this << Color::RED << "," << Color::RESET;
  ps->accept_fmt(this);
  if (ps->is_non_null_expr()) {
    *this << Color::RED << "," << Color::RESET;
    ps->accept_expr(this);
  }
  *this << Color::RED << ");" << Color::RESET;
}

void Printer::visit(const RestartStatement* rs) {
  *this << Color::YELLOW << "$restart" << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  rs->accept_arg(this);
  *this << Color::RED << ");" << Color::RESET;
}

void Printer::visit(const RetargetStatement* rs) {
  *this << Color::YELLOW << "$retarget" << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  rs->accept_arg(this);
  *this << Color::RED << ");" << Color::RESET;
}

void Printer::visit(const SaveStatement* ss) {
  *this << Color::YELLOW << "$save" << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  ss->accept_arg(this);
  *this << Color::RED << ");" << Color::RESET;
}

void Printer::visit(const WhileStatement* ws) {
  *this << Color::GREEN << "while " << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  ws->accept_cond(this); 
  *this << Color::RED << ") " << Color::RESET;
  ws->accept_stmt(this);
}

void Printer::visit(const EventControl* ec) {
  *this << Color::RED << "@" << Color::RESET;
  if (ec->empty_events()) {
    *this << Color::RED << "*" << Color::RESET;
    return;
  }
  *this << Color::RED << "(" << Color::RESET;
  int cnt = 0;
  ec->accept_events(this, [this,&cnt]{if (cnt++) {*this << Color::RED << " or " << Color::RESET;}}, []{});
  *this << Color::RED << ")" << Color::RESET;
}

void Printer::visit(const VariableAssign* va) {
  va->accept_lhs(this);
  *this << Color::RED << " = " << Color::RESET;
  va->accept_rhs(this);
}

Printer& Printer::operator<<(Color c) {
  switch (c) {
    case Color::RESET: 
      os_ << reset();
      break;
    case Color::RED:
      os_ << red();
      break;
    case Color::GREEN:
      os_ << green();
      break;
    case Color::YELLOW:
      os_ << yellow();
      break;
    case Color::BLUE:
      os_ << blue();
      break;
    case Color::GREY:
      os_ << grey();
      break;
    default:
      assert(false);
  }
  return *this;
}

Printer& Printer::operator<<(Node* n) {
  n->accept(this);
  return *this;
}

Printer& Printer::operator<<(const Node* n) {
  n->accept(this);
  return *this;
}

Printer& Printer::operator<<(uint64_t n) {
  os_ << n;
  return *this;
}

Printer& Printer::operator<<(const string& s) {
  os_ << s;
  return *this;
}

} // namespace cascade
