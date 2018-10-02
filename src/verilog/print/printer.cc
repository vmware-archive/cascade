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

#include "src/verilog/print/printer.h"

#include <array>
#include <cassert>
#include "src/verilog/ast/ast.h"

using namespace std;

namespace cascade {

Printer::Printer(ostream& os) : os_(os) { 
  os_.delim("\t");
}

void Printer::visit(const ArgAssign* aa) {
  if (!aa->get_exp()->null()) {
    *this << Color::RED << "." << Color::RESET;
    aa->get_exp()->accept(this); 
    *this << Color::RED << "(" << Color::RESET;
  }
  aa->get_imp()->accept(this);
  if (!aa->get_exp()->null()) {
    *this << Color::RED << ")" << Color::RESET;
  } 
}

void Printer::visit(const Attributes* as) {
  if (as->get_as()->empty()) {
    return;
  }
  *this << Color::RED << "(*" << Color::RESET;
  int cnt = 0;
  as->get_as()->accept(this, [this,&cnt]{if (cnt++) *this << Color::RED << "," << Color::RESET;}, []{});
  *this << Color::RED << "*) " << Color::RESET;
}

void Printer::visit(const AttrSpec* as) {
  as->get_lhs()->accept(this);
  if (!as->get_rhs()->null()) {
    *this << Color::RED << " = " << Color::RESET;
    as->get_rhs()->get()->accept(this);
  }
}

void Printer::visit(const CaseGenerateItem* cgi) {
  if (cgi->get_exprs()->empty()) {
    *this << Color::GREEN << "default" << Color::RESET;
  } 
  int cnt = 0;
  cgi->get_exprs()->accept(this, [this,&cnt]{if (cnt++) *this << Color::RED << ",\n" << Color::RESET;}, []{});
  *this << Color::RED << ": " << Color::RESET;
  cgi->get_block()->accept(this);
}

void Printer::visit(const CaseItem* ci) {
  if (ci->get_exprs()->empty()) {
    *this << Color::GREEN << "default" << Color::RESET;
  } 
  int cnt = 0;
  ci->get_exprs()->accept(this, [this,&cnt]{if (cnt++) *this << Color::RED << ",\n" << Color::RESET;}, []{});
  *this << Color::RED << ": " << Color::RESET;
  ci->get_stmt()->accept(this);
}

void Printer::visit(const Event* e) {
  static array<string,3> ets_ {{"","negedge ","posedge "}};
  *this << Color::GREEN << ets_[(size_t)e->get_type()] << Color::RESET;
  e->get_expr()->accept(this);
}

void Printer::visit(const BinaryExpression* be) {
  static array<string,24> ops_ {{
    "+",  "-",   "*",  "/",  "%", "===", "==",  "!==", "!=", "&&",
    "||", "**",  "<",  "<=", ">", ">=",  "&",   "|",   "^",  "~^",
    "<<", "<<<", ">>", ">>>"
  }};
  be->get_lhs()->accept(this);
  *this << " ";
  *this << Color::RED << ops_[(size_t)be->get_op()] << Color::RESET;
  *this << " ";
  be->get_rhs()->accept(this);
}

void Printer::visit(const ConditionalExpression* ce) {
  ce->get_cond()->accept(this);
  *this << Color::RED << " ? " << Color::RESET;
  ce->get_lhs()->accept(this);
  *this << Color::RED << " : " << Color::RESET;
  ce->get_rhs()->accept(this);
}

void Printer::visit(const NestedExpression* ne) {
  *this << Color::RED << "(" << Color::RESET;
  ne->get_expr()->accept(this);
  *this << Color::RED << ")" << Color::RESET;
}

void Printer::visit(const Concatenation* c) {
  *this << Color::RED << "{" << Color::RESET;
  int cnt = 0;
  c->get_exprs()->accept(this, [this,&cnt]{if (cnt++) *this << Color::RED << "," << Color::RESET;}, []{});
  *this << Color::RED << "}" << Color::RESET;
}

void Printer::visit(const Identifier* id) {
  int cnt = 0;
  id->get_ids()->accept(this, [this,&cnt]{if (cnt++) *this << ".";}, []{});
  if (!id->get_dim()->null()) {
    *this << Color::RED << "[" << Color::RESET;
    id->get_dim()->accept(this);
    *this << Color::RED << "]" << Color::RESET;
  }
}

void Printer::visit(const MultipleConcatenation* mc) {
  *this << Color::RED << "{" << Color::RESET;
  mc->get_expr()->accept(this);
  *this << " ";
  mc->get_concat()->accept(this);
  *this << Color::RED << "}" << Color::RESET;
}

void Printer::visit(const Number* n) {
  *this << Color::BLUE;
  if (n->get_format() != Number::UNBASED) {
    *this << n->get_val().size() << "'"; 
  }
  switch (n->get_format()) {
    case Number::UNBASED:
      n->get_val().write(os_, 10);
      break;
    case Number::BIN:
      *this << "b";
      n->get_val().write(os_, 2);
      break;
    case Number::OCT:
      *this << "o";
      n->get_val().write(os_, 8);
      break;
    case Number::HEX:
      *this << "h";
      n->get_val().write(os_, 16);
      break;
    default:
      *this << "d";
      n->get_val().write(os_, 10);
      break;
  } 
  *this << Color::RESET;
}

void Printer::visit(const String* s) {
  *this << Color::BLUE << "\"" << s->get_readable_val() << "\"" << Color::RESET;
}

void Printer::visit(const RangeExpression* re) {
  static array<string,3> sts_ {{":","+:","-:"}};
  re->get_upper()->accept(this);
  *this << Color::RED << sts_[(size_t)re->get_type()] << Color::RESET;
  re->get_lower()->accept(this);
}

void Printer::visit(const UnaryExpression* ue) {
  static array<string,10> ops_ {{"+",  "-",  "!",  "~",  "&", "~&", "|",  "~|", "^", "~^"}};
  *this << Color::RED << ops_[(size_t)ue->get_op()] << Color::RESET;
  ue->get_lhs()->accept(this);
}

void Printer::visit(const GenerateBlock* gb) {
  *this << Color::GREEN << "begin" << Color::RESET;
  if (!gb->get_id()->null()) {
    *this << Color::RED << " : " << Color::RESET;
    gb->get_id()->accept(this);
  }
  os_.tab();
  *this << "\n";
  gb->get_items()->accept(this, []{}, [this]{*this << "\n";});
  os_.untab();
  *this << Color::GREEN << "end " << Color::RESET;
}

void Printer::visit(const Id* id) {
  *this << id->get_readable_sid();
  if (!id->get_isel()->null()) {
    *this << Color::RED << "[" << Color::RESET;
    id->get_isel()->accept(this);
    *this << Color::RED << "]" << Color::RESET;
  }
}

void Printer::visit(const ModuleDeclaration* md) {
  md->get_attrs()->accept(this);
  if (!md->get_attrs()->get_as()->empty()) {
    *this << "\n";
  }
  *this << Color::GREEN << "module " << Color::RESET;
  md->get_id()->accept(this);
  *this << Color::RED << "(" << Color::RESET;
  int cnt = 0;
  md->get_ports()->accept(this, [this,&cnt]{if (cnt++) *this << Color::RED << "," << Color::RESET;}, []{});
  *this << Color::RED << ");" << Color::RESET << "\n";
  os_.tab();
  md->get_items()->accept(this, []{}, [this]{*this << "\n";});
  os_.untab();
  *this << Color::GREEN << "endmodule" << Color::RESET;
}

void Printer::visit(const IfGenerateConstruct* igc) {
  igc->get_attrs()->accept(this);
  if (!igc->get_attrs()->get_as()->empty()) {
    *this << "\n";
  }
  *this << Color::GREEN << "if " << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  igc->get_if()->accept(this);
  *this << Color::RED << ") " << Color::RESET;
  igc->get_then()->accept(this);
  if (!igc->get_else()->null()) {
    *this << Color::GREEN << "else " << Color::RESET;
    igc->get_else()->accept(this);
  }
}

void Printer::visit(const AlwaysConstruct* ac) {
  *this << Color::GREEN << "always " << Color::RESET;
  ac->get_stmt()->accept(this);
}

void Printer::visit(const CaseGenerateConstruct* cgc) {
  *this << Color::GREEN << "case" << Color::RESET;
  *this << Color::RED << " (" << Color::RESET;
  cgc->get_cond()->accept(this);
  *this << Color::RED << ")" << Color::RESET << "\n";
  os_.tab();
  cgc->get_items()->accept(this, []{}, [this]{*this << "\n";});
  os_.untab();
  *this << Color::GREEN << "endcase" << Color::RESET;
}

void Printer::visit(const LoopGenerateConstruct* lgc) {
  *this << Color::GREEN << "for " << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  lgc->get_init()->accept(this);
  *this << Color::RED << "; " << Color::RESET;
  lgc->get_cond()->accept(this); 
  *this << Color::RED << "; " << Color::RESET;
  lgc->get_update()->accept(this);
  *this << Color::RED << ") " << Color::RESET;
  lgc->get_block()->accept(this);
}

void Printer::visit(const InitialConstruct* ic) {
  ic->get_attrs()->accept(this);
  if (!ic->get_attrs()->get_as()->empty()) {
    *this << "\n";
  }
  *this << Color::GREEN << "initial " << Color::RESET;
  ic->get_stmt()->accept(this);
}

void Printer::visit(const ContinuousAssign* ca) {
  *this << Color::GREEN << "assign " << Color::RESET;
  ca->get_ctrl()->accept(this, []{}, [this]{*this << " ";});
  ca->get_assign()->accept(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const GenvarDeclaration* gd) {
  gd->get_attrs()->accept(this);
  *this << Color::GREEN << "genvar " << Color::RESET;
  gd->get_id()->accept(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const IntegerDeclaration* id) {
  id->get_attrs()->accept(this);
  *this << Color::GREEN << "integer " << Color::RESET;
  id->get_id()->accept(this);
  if (!id->get_val()->null()) {
    *this << Color::RED << " = " << Color::RESET;
    id->get_val()->get()->accept(this);
  }
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const LocalparamDeclaration* ld) {
  ld->get_attrs()->accept(this);
  *this << Color::GREEN << "localparam" << Color::RESET;
  if (ld->get_signed()) {
    *this << Color::GREEN << " signed" << Color::RESET;
  }
  if (!ld->get_dim()->null()) {
    *this << Color::RED << "[" << Color::RESET;
    ld->get_dim()->accept(this);
    *this << Color::RED << "]" << Color::RESET;
  }
  *this << " ";
  ld->get_id()->accept(this);
  *this << Color::RED << " = " << Color::RESET;
  ld->get_val()->accept(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const NetDeclaration* nd) {
  nd->get_attrs()->accept(this);
  static array<string,1> nts_ {{"wire"}};
  *this << Color::GREEN << nts_[(size_t)nd->get_type()] << Color::RESET;
  if (nd->get_signed()) {
    *this << Color::GREEN << " signed" << Color::RESET;
  }
  if (!nd->get_dim()->null()) {
    *this << Color::RED << "[" << Color::RESET;
    nd->get_dim()->accept(this);
    *this << Color::RED << "]" << Color::RESET;
  }
  *this << " ";
  nd->get_ctrl()->accept(this, []{}, [this]{*this << " ";});
  nd->get_id()->accept(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const ParameterDeclaration* pd) {
  pd->get_attrs()->accept(this);
  *this << Color::GREEN << "parameter" << Color::RESET;
  if (pd->get_signed()) {
    *this << Color::GREEN << " signed" << Color::RESET;
  }
  if (!pd->get_dim()->null()) {
    *this << Color::RED << "[" << Color::RESET;
    pd->get_dim()->accept(this);
    *this << Color::RED << "]" << Color::RESET;
  }
  *this << " ";
  pd->get_id()->accept(this);
  *this << Color::RED << " = " << Color::RESET;
  pd->get_val()->accept(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const RegDeclaration* rd) {
  rd->get_attrs()->accept(this);
  *this << Color::GREEN << "reg" << Color::RESET;
  if (rd->get_signed()) {
    *this << Color::GREEN << " signed" << Color::RESET;
  }
  if (!rd->get_dim()->null()) {
    *this << Color::RED << "[" << Color::RESET;
    rd->get_dim()->accept(this);
    *this << Color::RED << "]" << Color::RESET;
  }
  *this << " ";
  rd->get_id()->accept(this);
  if (!rd->get_val()->null()) {
    *this << Color::RED << " = " << Color::RESET;
    rd->get_val()->get()->accept(this);
  }
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const GenerateRegion* gr) {
  *this << Color::GREEN << "generate" << Color::RESET << "\n";
  os_.tab();
  gr->get_items()->accept(this, []{}, [this]{*this << "\n";});
  os_.untab();
  *this << Color::GREEN << "endgenerate" << Color::RESET;
}

void Printer::visit(const ModuleInstantiation* mi) {
  mi->get_attrs()->accept(this);
  if (!mi->get_attrs()->get_as()->empty()) {
    *this << "\n";
  }
  mi->get_mid()->accept(this);
  *this << " ";
  if (!mi->get_params()->empty()) {
    *this << Color::RED << "#(" << Color::RESET;
    int cnt = 0;
    mi->get_params()->accept(this, [this,&cnt]{if (cnt++) *this << Color::RED << "," << Color::RESET;}, []{});
    *this << Color::RED << ") " << Color::RESET;
  }
  mi->get_iid()->accept(this);
  *this << Color::RED << "(" << Color::RESET;
  int cnt = 0;
  mi->get_ports()->accept(this, [this,&cnt]{if (cnt++) *this << Color::RED << "," << Color::RESET;}, []{});
  *this << Color::RED << ");" << Color::RESET;
}

void Printer::visit(const PortDeclaration* pd) { 
  static array<string,3> pts_ {{"inout","input","output"}};
  pd->get_attrs()->accept(this);
  *this << Color::GREEN << pts_[(size_t)pd->get_type()] << Color::RESET;
  *this << " ";
  pd->get_decl()->accept(this);
}

void Printer::visit(const BlockingAssign* ba) {
  ba->get_assign()->get_lhs()->accept(this);
  *this << Color::RED << " = " << Color::RESET;
  ba->get_ctrl()->accept(this, []{}, [this]{*this << " ";});
  ba->get_assign()->get_rhs()->accept(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const NonblockingAssign* na) {
  na->get_assign()->get_lhs()->accept(this);
  *this << Color::RED << " <= " << Color::RESET;
  na->get_ctrl()->accept(this, []{}, [this]{*this << " ";});
  na->get_assign()->get_rhs()->accept(this);
  *this << Color::RED << ";" << Color::RESET;
}

void Printer::visit(const CaseStatement* cs) {
  static array<string,3> cts_ {{"case","casex","casez"}};
  *this << Color::GREEN << cts_[(size_t)cs->get_type()] << Color::RESET;
  *this << Color::RED << " (" << Color::RESET;
  cs->get_cond()->accept(this);
  *this << Color::RED << ")" << Color::RESET << "\n";
  os_.tab();
  cs->get_items()->accept(this, []{}, [this]{*this << "\n";});
  os_.untab();
  *this << Color::GREEN << "endcase" << Color::RESET;
}

void Printer::visit(const ConditionalStatement* cs) {
  *this << Color::GREEN << "if " << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  cs->get_if()->accept(this);
  *this << Color::RED << ") " << Color::RESET;

  const auto tpb = dynamic_cast<const ParBlock*>(cs->get_then());
  const auto tsb = dynamic_cast<const SeqBlock*>(cs->get_then());
  if ((tpb == nullptr) && (tsb == nullptr)) {
    os_.tab();
    *this << "\n";
    cs->get_then()->accept(this);
    os_.untab();
  } else {
    cs->get_then()->accept(this);
  }

  const auto epb = dynamic_cast<const ParBlock*>(cs->get_else());
  const auto esb = dynamic_cast<const SeqBlock*>(cs->get_else());
  if ((epb == nullptr) && (esb == nullptr)) {
    *this << "\n";
    *this << Color::GREEN << "else " << Color::RESET;
    os_.tab();
    *this << "\n";
    cs->get_else()->accept(this);
    os_.untab();
  } else if (((epb != nullptr) && !epb->get_stmts()->empty()) || ((esb != nullptr) && !esb->get_stmts()->empty())) {
    *this << "\n";
    *this << Color::GREEN << "else " << Color::RESET;
    cs->get_else()->accept(this);
  }
}

void Printer::visit(const ForStatement* fs) {
  *this << Color::GREEN << "for " << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  fs->get_init()->accept(this);
  *this << Color::RED << "; " << Color::RESET;
  fs->get_cond()->accept(this); 
  *this << Color::RED << "; " << Color::RESET;
  fs->get_update()->accept(this);
  *this << Color::RED << ") " << Color::RESET;
  fs->get_stmt()->accept(this);
}

void Printer::visit(const ForeverStatement* fs) {
  *this << Color::GREEN << "forever " << Color::RESET;
  fs->get_stmt()->accept(this);
}

void Printer::visit(const RepeatStatement* rs) {
  *this << Color::GREEN << "repeat " << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  rs->get_cond()->accept(this); 
  *this << Color::RED << ") " << Color::RESET;
  rs->get_stmt()->accept(this);
}

void Printer::visit(const ParBlock* pb) { 
  *this << Color::GREEN << "fork" << Color::RESET;
  if (!pb->get_id()->null()) {
    *this << Color::RED << " : " << Color::RESET;
    pb->get_id()->accept(this);
  }
  *this << "\n";
  os_.tab();
  pb->get_decls()->accept(this, []{}, [this]{*this << "\n";});
  pb->get_stmts()->accept(this, []{}, [this]{*this << "\n";});
  os_.untab();
  *this << Color::GREEN << "join " << Color::RESET;
}

void Printer::visit(const SeqBlock* sb) { 
  *this << Color::GREEN << "begin" << Color::RESET;
  if (!sb->get_id()->null()) {
    *this << Color::RED << " : " << Color::RESET;
    sb->get_id()->accept(this);
  }
  os_.tab();
  *this << "\n";
  sb->get_decls()->accept(this, []{}, [this]{*this << "\n";});
  sb->get_stmts()->accept(this, []{}, [this]{*this << "\n";});
  os_.untab();
  *this << Color::GREEN << "end " << Color::RESET;
}

void Printer::visit(const TimingControlStatement* ptc) {
  ptc->get_ctrl()->accept(this);
  *this << " ";
  ptc->get_stmt()->accept(this);
}

void Printer::visit(const DisplayStatement* ds) {
  *this << Color::YELLOW << "$display" << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  int cnt = 0;
  ds->get_args()->accept(this, [this,&cnt]{if (cnt++) *this << Color::RED << "," << Color::RESET;}, []{});
  *this << Color::RED << ");" << Color::RESET;
}

void Printer::visit(const FinishStatement* fs) {
  *this << Color::YELLOW << "$finish" << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  fs->get_arg()->accept(this);
  *this << Color::RED << ");" << Color::RESET;
}

void Printer::visit(const WriteStatement* ws) {
  *this << Color::YELLOW << "$write" << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  int cnt = 0;
  ws->get_args()->accept(this, [this,&cnt]{if (cnt++) *this << Color::RED << "," << Color::RESET;}, []{});
  *this << Color::RED << ");" << Color::RESET;
}

void Printer::visit(const WaitStatement* ws) {
  *this << Color::GREEN << "wait " << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  ws->get_cond()->accept(this); 
  *this << Color::RED << ")" << Color::RESET;
  *this << " ";
  ws->get_stmt()->accept(this);
}

void Printer::visit(const WhileStatement* ws) {
  *this << Color::GREEN << "while " << Color::RESET;
  *this << Color::RED << "(" << Color::RESET;
  ws->get_cond()->accept(this); 
  *this << Color::RED << ") " << Color::RESET;
  ws->get_stmt()->accept(this);
}

void Printer::visit(const DelayControl* dc) {
  *this << Color::RED << "#" << Color::RESET;
  dc->get_delay()->accept(this);
}

void Printer::visit(const EventControl* ec) {
  *this << Color::RED << "@" << Color::RESET;
  if (ec->get_events()->empty()) {
    *this << Color::RED << "*" << Color::RESET;
    return;
  }
  *this << Color::RED << "(" << Color::RESET;
  int cnt = 0;
  ec->get_events()->accept(this, [this,&cnt]{if (cnt++) *this << Color::RED << " or " << Color::RESET;}, []{});
  *this << Color::RED << ")" << Color::RESET;
}

void Printer::visit(const VariableAssign* va) {
  va->get_lhs()->accept(this);
  *this << Color::RED << " = " << Color::RESET;
  va->get_rhs()->accept(this);
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
