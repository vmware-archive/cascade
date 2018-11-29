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

#include "src/verilog/program/type_check.h"

#include <sstream>
#include "src/base/log/log.h"
#include "src/verilog/analyze/constant.h"
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/navigate.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/parse/parser.h"
#include "src/verilog/print/text/text_printer.h"
#include "src/verilog/program/elaborate.h"
#include "src/verilog/program/program.h"

using namespace std;

namespace cascade {

TypeCheck::TypeCheck(const Program* program, Log* log, const Parser* parser) : Visitor() { 
  program_ = program;
  log_ = log;
  parser_ = parser;

  deactivate(false);
  declaration_check(true);
  local_only(true);

  outermost_loop_ = nullptr;
  instantiation_ = nullptr;
  net_lval_ = false;

  exists_bad_id_ = false;
}

void TypeCheck::deactivate(bool val) {
  deactivated_ = val;
}

void TypeCheck::declaration_check(bool val) {
  decl_check_ = val;
}

void TypeCheck::local_only(bool val) {
  local_only_ = val;
}

void TypeCheck::pre_elaboration_check(const ModuleInstantiation* mi) {
  if (deactivated_) {
    return;
  }

  // RECURSE: Implicit params and ports
  const auto backup = exists_bad_id_;
  for (auto p : *mi->get_params()) {
    p->maybe_accept_imp(this);
  }
  for (auto p : *mi->get_ports()) {
    p->maybe_accept_imp(this);
  }
  // EXIT: Arity checking will fail without access to resolvable ports
  if (exists_bad_id_) {
    exists_bad_id_ = backup;
    return;
  }

  // CHECK: Array properties
  check_width(mi->get_range());

  // CHECK: Duplicate definition for instantiations other than the root
  if (!Navigate(mi).lost()) {
    if (Navigate(mi).find_duplicate_name(mi->get_iid()->get_ids()->back())) {
      multiple_def(mi->get_iid());
    }
    if (Navigate(mi).find_child(mi->get_iid()->get_ids()->back())) {
      error("A nested scope with this name already exists in this scope", mi);
    }
  }
  // CHECK: TODO Recursive instantiation

  // CHECK: Undefined type
  const auto itr = program_->decl_find(mi->get_mid());
  if (itr == program_->decl_end()) {
    if (decl_check_) {
      warn("Instantiation refers to an undeclared module, this may result in an error during elaboration", mi);
    } else {
      error("Instantiation refers to an undeclared module", mi);
    }
    // EXIT: Can't continue checking without access to source
    return;
  }
  const auto src = itr->second;

  // CHECK: More overrides than declared parameters
  if (mi->get_params()->size() > ModuleInfo(src).ordered_params().size()) {
    error("Instantiation contains more parameter overrides than appear in module declaration", mi);
  }
  // CHECK: Duplicate or unrecognized parameters
  if (mi->uses_named_params()) {
    const auto& np = ModuleInfo(src).named_params();
    unordered_set<const Identifier*, HashId, EqId> params;
    for (auto p : *mi->get_params()) {
      if (!params.insert(p->get_exp()).second) {
        error("Instantiation contains duplicate named params", mi);
      }
      if (np.find(p->get_exp()) == np.end()) {
        error("Instantiation contains a reference to unresolvable parameter", mi);
      }
    }
  }

  // CHECK: More connections than declared ports
  if (mi->get_ports()->size() > ModuleInfo(src).ordered_ports().size()) {
    error("Instantiation contains more connections than appear in module declaration", mi);
  }
  // CHECK: Duplicate or unrecognized port connections 
  // CHECK: expressions connected to outputs
  // CHECK: Arity mismatch for instance arrays
  if (mi->uses_named_ports()) {
    const auto& np = ModuleInfo(src).named_ports();
    unordered_set<const Identifier*, HashId, EqId> ports;
    for (auto p : *mi->get_ports()) {
      if (!ports.insert(p->get_exp()).second) {
        error("Instantiation contains duplicate named connections", mi);
      }
      const auto itr = np.find(p->get_exp());
      if (itr == np.end()) {
        error("Instantiation contains a reference to an unresolvable explicit port", mi);
        continue;
      }
      if (p->is_null_imp()) {
        continue;
      }
      if (ModuleInfo(src).is_output(*itr)) {
        if (dynamic_cast<const Identifier*>(p->get_imp()) == nullptr) {
          error("Instantiation contains a connection between an expression and a named output port", mi);
        }
      }
      check_arity(mi, *itr, p->get_imp());
    }
  } 
  if (!mi->uses_named_ports()) {
    const auto& op = ModuleInfo(src).ordered_ports();
    for (size_t i = 0, ie = min(op.size(), mi->get_ports()->size()); i < ie; ++i) {
      const auto p = mi->get_ports()->get(i);
      if (p->is_null_imp()) {
        continue;
      }
      if (ModuleInfo(src).is_output(op[i])) {
        if (dynamic_cast<const Identifier*>(p->get_imp()) == nullptr) {
          error("Instantiation contains a connection between an expression and an ordered output port", mi);
        }
      }
      check_arity(mi, op[i], p->get_imp());
    }
  }
}

void TypeCheck::pre_elaboration_check(const CaseGenerateConstruct* cgc) {
  if (deactivated_) {
    return;
  }
  // CHECK: Constant guard expression 
  if (!Constant().is_static_constant(cgc->get_cond())) {
    error("Non-constant expression appears in the guard for a case generate construct", cgc->get_cond());
  }
  // RECURSE: condition
  cgc->get_cond()->accept(this);
}

void TypeCheck::pre_elaboration_check(const IfGenerateConstruct* igc) {
  if (deactivated_) {
    return;
  }
  // CHECK: Constant guard expression 
  for (auto c : *igc->get_clauses()) {
    if (!Constant().is_static_constant(c->get_if())) {
      error("Non-constant expression appears in the guard for an if generate construct", c->get_if());
    }
    // RECURSE: condition
    c->get_if()->accept(this);
  }
}

void TypeCheck::pre_elaboration_check(const LoopGenerateConstruct* lgc) {
  // If this is the outermost loop, set the location
  if (outermost_loop_ == nullptr) {
    outermost_loop_ = lgc;
  }

  if (!deactivated_) {
    // RECURSE: Iteration space
    lgc->get_init()->accept(this);
    lgc->get_cond()->accept(this);
    lgc->get_update()->accept(this);

    // CHECK: Const loop guard
    if (!Constant().is_genvar_constant(lgc->get_cond())) {
      error("Non-constant expression appears in the guard for a loop generate construct", lgc->get_cond());
    }
    // CHECK: Initialization and update refer to same variable
    const auto r1 = Resolve().get_resolution(lgc->get_init()->get_lhs());
    const auto r2 = Resolve().get_resolution(lgc->get_update()->get_lhs());
    if (r1 != r2) {
      error("Initialization and update statements refer to different variables in loop generate construct", lgc->get_update()->get_lhs());
    }
  }

  // If this is the outermost loop, we're about to emerge from it
  if (outermost_loop_ == lgc) {
    outermost_loop_ = nullptr;
  }
}

void TypeCheck::post_elaboration_check(const Node* n) {
  if (deactivated_) {
    return;
  }
  n->accept(this);
}

void TypeCheck::warn(const string& s, const Node* n) {
  stringstream ss;

  auto ptr = n;
  if (decl_check_) {
    TextPrinter(ss) << "In module declaration ";
  } else if (instantiation_ != nullptr) {
    TextPrinter(ss) << "In module instantiation ";
    ptr = instantiation_;
  } else {
    TextPrinter(ss) << "In module item ";
  }

  if (parser_ == nullptr) {
    TextPrinter(ss) << "in <unable to access location --- contact developers>: ";
  } else {
    const auto loc = parser_->get_loc(ptr);
    if (loc.first == "<top>") {
      TextPrinter(ss) << "in final line of user input: ";
    } else {
      TextPrinter(ss) << "in " << loc.first << " on line " << loc.second << ": ";
    }
  }
  
  TextPrinter(ss) << ptr << "\n" << s;
  if (ptr != n) {
    TextPrinter(ss) << ", see previous warnings for more information";
  }

  log_->warn(ss.str());
}

void TypeCheck::error(const string& s, const Node* n) {
  stringstream ss;

  auto ptr = n;
  if (decl_check_) {
    TextPrinter(ss) << "In module declaration ";
  } else if (instantiation_ != nullptr) {
    TextPrinter(ss) << "In module instantiation ";
    ptr = instantiation_;
  } else {
    TextPrinter(ss) << "In module item ";
  }

  if (parser_ == nullptr) {
    TextPrinter(ss) << "in <unable to access location --- contact developers>: ";
  } else {
    const auto loc = parser_->get_loc(ptr);
    if (loc.first == "<top>") {
      TextPrinter(ss) << "in final line of user input: ";
    } else {
      TextPrinter(ss) << "in " << loc.first << " on line " << loc.second << ": ";
    }
  }
  
  TextPrinter(ss) << ptr << "\n" << s;
  if (ptr != n) {
    TextPrinter(ss) << ", see previous warnings for more information";
  }

  log_->error(ss.str());
}

void TypeCheck::multiple_def(const Node* n) {
  error("A variable with this name already appears in this scope", n);
}

void TypeCheck::visit(const Identifier* id) {
  // CHECK: Are instance selects constant expressions?
  for (auto i : *id->get_ids()) {
    if (i->is_non_null_isel() && !Constant().is_static_constant(i->get_isel())) {
      error("Found non-constant expression in instance select", id);
    }
  }

  // RECURSE: ids and dim
  auto backup = exists_bad_id_;
  id->get_ids()->accept(this);
  id->get_dim()->accept(this);
  // EXIT: Resolution will fail if there's a bad id below here
  if (exists_bad_id_) {
    exists_bad_id_ = backup;
    return;
  }

  // CHECK: Reference to undefined identifier
  const auto r = Resolve().get_resolution(id);
  if (r == nullptr) {
    exists_bad_id_ = true;
    if (decl_check_) {
      warn("Found reference to unresolvable identifier, this may result in an error during instantiation", id);
    } else {
      error("Found reference to an unresolvable identifier", id);
    }
  }
  // CHECK: Is this a reference to a genvar outside of a loop generate construct?
  if (outermost_loop_ == nullptr) {
    if ((r != nullptr) && (dynamic_cast<const GenvarDeclaration*>(r->get_parent()) != nullptr)) {
      error("Found reference to a genvar outside of a loop generate construct", id);
    }
  }
  // EXIT: Can't continue checking if id can't be resolved
  if (r == nullptr) {
    return;
  }

  // CHECK: Are subscripts valid, if provided?
  const auto cdr = check_deref(r, id);
  // Nothing else to do if we're out of dimensions
  if (cdr == id->get_dim()->end()) {
    return;
  }

  // CHECK: Are we providing a bit-select for a single-bit value?
  if (Evaluate().get_width(r) == 1) {
    return error("Found bit- or part-select in dereference of variable which was declared scalar", id);
  }

  // CHECK: Range expression bit-selects
  if (const auto re = dynamic_cast<const RangeExpression*>(*cdr)) {
    if (re->get_type() == RangeExpression::CONSTANT) {
      // CHECK: Non-constant values, values out of range, little-endian ranges
      // EXIT: We can't continue checking if we can't evaluate this range
      if (!Constant().is_static_constant(re)) {
        return error("Found non-constant value in constant part-select", id);
      }
      const auto rng = Evaluate().get_range(re);
      if (rng.first < rng.second) {
        return error("Cascade does not currently support big-endian part-selects", id);
      } 
    } else {
      if (!Constant().is_static_constant(re->get_lower())) {
        return error("Found non-constant width in indexed part-select", id);
      }
    }
  }
  if (!Constant().is_static_constant(*cdr)) {
    // ERROR: Is this a non-constant in a net_lval?
    if (net_lval_) {
      error("Found non-constant bit- or part-select in target of continuous assignment", *cdr);
    }
  }
  else {
    // WARN: Can we say for sure that selects are out of range
    const auto rng = Evaluate().get_range(*cdr);
    if ((rng.first >= Evaluate().get_width(r)) || (rng.second >= Evaluate().get_width(r))) {
      if (!decl_check_) {
        warn("Found bit- or part-select which is out of range of declared width for this variable", id);
      }
    }
  } 
}

void TypeCheck::visit(const String* s) {
  auto e = false;
  if (auto ws = dynamic_cast<const WriteStatement*>(s->get_parent()->get_parent())) {
    if (ws->get_args()->front() != s) {
      e = true;
    }
  } else if (auto ds = dynamic_cast<const DisplayStatement*>(s->get_parent()->get_parent())) {
    if (ds->get_args()->front() != s) {
      e = true;
    }
  } else {
    e = true;
  }
  if (e) {
    error("Cascade does not currently support the use of string constants in expressions", s);
  }
}

void TypeCheck::visit(const GenerateBlock* gb) {
  // TODO CHECK: Duplicate definition
  // RECURSE: items
  gb->get_items()->accept(this);
}

void TypeCheck::visit(const ModuleDeclaration* md) {
  // CHECK: implicit or explict ports that are not simple ids
  for (auto p : *md->get_ports()) {
    if (p->is_non_null_exp()) {
      error("Cascade does not currently support the use of explicit ports in module declarations", p);
    }
    if (p->is_null_imp()) {
      error("Found a missing implicit port in module declaration", md);
    }
    auto imp = dynamic_cast<const Identifier*>(p->get_imp());
    if (imp == nullptr) {
      error("Cascade does not currently support the use of implicit ports which are not identifiers", p);
    } else {
      // RECURSE: implicit port
      imp->accept(this);
    }
  }

  // RECURSE: items
  md->get_items()->accept(this);
}

void TypeCheck::visit(const CaseGenerateConstruct* cgc) {
  if (local_only_) {
    return;
  }
  assert(Elaborate().is_elaborated(cgc));
  auto gen = Elaborate().get_elaboration(cgc);
  gen->accept(this);
}

void TypeCheck::visit(const IfGenerateConstruct* igc) {
  if (local_only_) {
    return;
  }
  assert(Elaborate().is_elaborated(igc));
  auto gen = Elaborate().get_elaboration(igc);
  gen->accept(this);
}

void TypeCheck::visit(const LoopGenerateConstruct* lgc) {
  if (local_only_) {
    return;
  }
  assert(Elaborate().is_elaborated(lgc));
  auto gen = Elaborate().get_elaboration(lgc);
  gen->accept(this);
}

void TypeCheck::visit(const InitialConstruct* ic) {
  // RECURSE: body
  ic->get_stmt()->accept(this);
}

void TypeCheck::visit(const ContinuousAssign* ca) {
  ca->maybe_accept_ctrl(this);
  net_lval_ = true;
  ca->get_assign()->get_lhs()->accept(this);
  net_lval_ = false;
  ca->get_assign()->get_rhs()->accept(this);

  const auto r = Resolve().get_resolution(ca->get_assign()->get_lhs());
  if ((r != nullptr) && (dynamic_cast<const NetDeclaration*>(r->get_parent()) == nullptr)) {
    error("Continuous assignments are only permitted for variables with type wire", ca);
  }
}

void TypeCheck::visit(const GenvarDeclaration* gd) {
  // CHECK: Duplicate definition
  if (Navigate(gd).find_duplicate_name(gd->get_id()->get_ids()->back())) {
    multiple_def(gd->get_id());
  }
  if (Navigate(gd).find_child(gd->get_id()->get_ids()->back())) {
    error("A nested scope scope with this name already exists in this scope", gd);
  }
}

void TypeCheck::visit(const IntegerDeclaration* id) {
  // RECURSE: Check for unsupported language features in initial value
  id->maybe_accept_val(this);

  // CHECK: Duplicate definition
  if (Navigate(id).find_duplicate_name(id->get_id()->get_ids()->back())) {
    multiple_def(id->get_id());
  }
  if (Navigate(id).find_child(id->get_id()->get_ids()->back())) {
    error("A nested scope scope with this name already exists in this scope", id);
  }
  // CHECK: Integer initialized to constant value
  if (id->is_non_null_val() && !Constant().is_static_constant(id->get_val())) {
    error("Integer initialization requires constant value", id);
  }
  // CHECK: Array properties
  check_array(id->get_id()->get_dim());
}

void TypeCheck::visit(const LocalparamDeclaration* ld) {
  // RECURSE: Check for unsupported language features in initial value
  ld->get_val()->accept(this);

  // CHECK: Duplicate definition
  if (Navigate(ld).find_duplicate_name(ld->get_id()->get_ids()->back())) {
    multiple_def(ld->get_id());
  }
  if (Navigate(ld).find_child(ld->get_id()->get_ids()->back())) {
    error("A nested scope scope with this name already exists in this scope", ld);
  }
  // CHECK: Width properties
  check_width(ld->get_dim());
  // CHECK: Parameter initialized to constant value
  if (!Constant().is_static_constant(ld->get_val())) {
    error("Localparam initialization requires constant value", ld);
  }
}

void TypeCheck::visit(const NetDeclaration* nd) {
  // CHECK: Duplicate definition
  if (Navigate(nd).find_duplicate_name(nd->get_id()->get_ids()->back())) {
    multiple_def(nd->get_id());
  }
  if (Navigate(nd).find_child(nd->get_id()->get_ids()->back())) {
    error("A nested scope scope with this name already exists in this scope", nd);
  }
  // CHECK: Width and array properties
  check_width(nd->get_dim());
  check_array(nd->get_id()->get_dim());
  // CHECK: Delay control statements
  if (nd->is_non_null_ctrl()) {
    error("No support for delay control statements in net declarations", nd);
  }
}

void TypeCheck::visit(const ParameterDeclaration* pd) {
  // RECURSE: Check for unsupported language features in initial value
  pd->get_val()->accept(this);

  // CHECK: Duplicate definition
  if (Navigate(pd).find_duplicate_name(pd->get_id()->get_ids()->back())) {
    multiple_def(pd->get_id());
  }
  if (Navigate(pd).find_child(pd->get_id()->get_ids()->back())) {
    error("A nested scope scope with this name already exists in this scope", pd);
  }
  // CHECK: Width properties
  check_width(pd->get_dim());
  // CHECK: Parameter initialized to constant value
  if (!Constant().is_static_constant(pd->get_val())) {
    error("Parameter initialization requires constant value", pd);
  }
}

void TypeCheck::visit(const RegDeclaration* rd) {
  // RECURSE: Check for unsupported language features in initial value
  rd->maybe_accept_val(this);

  // CHECK: Duplicate definition
  if (Navigate(rd).find_duplicate_name(rd->get_id()->get_ids()->back())) {
    multiple_def(rd->get_id());
  }
  if (Navigate(rd).find_child(rd->get_id()->get_ids()->back())) {
    error("A nested scope scope with this name already exists in this scope", rd);
  }
  // CHECK: Width and array properties
  check_width(rd->get_dim());
  check_array(rd->get_id()->get_dim());
  // CHECK: Registers initialized to constant value
  if (rd->is_non_null_val() && !Constant().is_static_constant(rd->get_val())) {
    error("Register initialization requires constant value", rd);
  }
}

void TypeCheck::visit(const ModuleInstantiation* mi) {
  if (local_only_) {
    return;
  }
  assert(Elaborate().is_elaborated(mi));
  auto inst = Elaborate(program_).get_elaboration(mi);
  instantiation_ = mi;
  inst->accept(this);
  instantiation_ = nullptr;
}

void TypeCheck::visit(const ParBlock* pb) {
  // CHECK: TODO Duplicate definition
  // RECURSE: decls and body
  pb->get_decls()->accept(this);
  pb->get_stmts()->accept(this);
}

void TypeCheck::visit(const SeqBlock* sb) {
  // CHECK: TODO Duplicate definition
  // RECURSE: decls and body
  sb->get_decls()->accept(this);
  sb->get_stmts()->accept(this);
}

void TypeCheck::visit(const BlockingAssign* ba) {
  // RECURSE: 
  Visitor::visit(ba);
  // CHECK: Target must be register or integer
  const auto r = Resolve().get_resolution(ba->get_assign()->get_lhs());
  if ((r != nullptr) && 
      (dynamic_cast<const RegDeclaration*>(r->get_parent()) == nullptr) && 
      (dynamic_cast<const IntegerDeclaration*>(r->get_parent()) == nullptr)) {
    error("Found a blocking assignments to a variable with type other than reg or integer", ba);
  }
}

void TypeCheck::visit(const NonblockingAssign* na) {
  // RECURSE:
  Visitor::visit(na);
  // CHECK: Target must be register or integer
  const auto r = Resolve().get_resolution(na->get_assign()->get_lhs());
  if ((r != nullptr) && 
      (dynamic_cast<const RegDeclaration*>(r->get_parent()) == nullptr) && 
      (dynamic_cast<const IntegerDeclaration*>(r->get_parent()) == nullptr)) {
    error("Found a non-blocking assignments to a variable with type other than reg or integer", na);
  }
}

void TypeCheck::visit(const ForStatement* fs) {
  error("Cascade does not currently support the use of for statements", fs);
}

void TypeCheck::visit(const ForeverStatement* fs) {
  error("Cascade does not currently support the use of forever statements", fs);
}

void TypeCheck::visit(const RepeatStatement* rs) {
  error("Cascade does not currently support the use of repeat statements", rs);
}

void TypeCheck::visit(const WhileStatement* ws) {
  error("Cascade does not currently support the use of while statements", ws);
}

void TypeCheck::visit(const WaitStatement* ws) {
  error("Cascade does not currently support the use of wait statements", ws);
}

void TypeCheck::visit(const DelayControl* dc) {
  error("Cascade does not currently support the use of delay controls", dc);
}

void TypeCheck::check_width(const RangeExpression* re) {
  if (re == nullptr) {
    return;
  }

  // RECURSE: First check the contents of this expression
  const auto backup = exists_bad_id_;
  re->accept(this);
  // EXIT: Evaluation will fail if there's a bad id below here
  if (exists_bad_id_) {
    exists_bad_id_ = backup;
    return;
  }

  const auto rng = Evaluate().get_range(re);
  if (rng.first <= rng.second) {
    error("Cascade does not currently support little-endian vector declarations", re);
  }
  if (rng.second != 0) {
    error("Cascade does not currently support vector declarations with lower bounds not equal to zero", re);
  }
}

void TypeCheck::check_array(const Many<Expression>* es) {
  for (auto e : *es) {
    // CHECK: Array bounds must be constants
    if (!Constant().is_static_constant(e)) {
      return error("Found a non-constant expression in an array declaration", es->get_parent()->get_parent());
    }
    // RECURSE: Check the contents of this array bound
    const auto backup = exists_bad_id_;
    e->accept(this);
    // EXIT: Evaluation will fail if there's a bad id below here
    if (exists_bad_id_) {
      exists_bad_id_ = backup;
      return;
    }
   
    const auto rng = Evaluate().get_range(e);
    if (rng.first <= rng.second) {
      return error("Cascade does not currently support little-endian array declarations", es->get_parent()->get_parent());
    } 
    if (rng.second != 0) {
      return error("Cascade does not currently support array declarations with lower bounds not equal to zero", es->get_parent()->get_parent());
    }
  }
}

Many<Expression>::const_iterator TypeCheck::check_deref(const Identifier* r, const Identifier* i) {
  const int diff = i->get_dim()->size() - r->get_dim()->size();

  // We need to have at least as many subscripts on i as r is declared with
  if (diff < 0) {
    error("Found an array dereference with more subscripts than appear in the declaration for this variable", i);
    return i->get_dim()->end();
  }
  // If we have the same number, the last subscript on i can't be a range
  // There's also nothing to do if the subscripts are both empty
  else if (diff == 0) {
    if (i->get_dim()->empty()) {
      return i->get_dim()->end();
    } else if (dynamic_cast<const RangeExpression*>(i->get_dim()->back())) {
      error("Found a range expression found where a scalar subscript was expected", i);
      return i->get_dim()->end();
    }
  }
  // And if i has more than one too many, that's an error as well
  else if (diff > 1) {
    error("Found an array dereference with more subscripts than appear in the declaration for this variable", i);
    return i->get_dim()->end();
  } 
  
  // Iterate over subscripts
  auto iitr = i->get_dim()->begin();
  auto ritr = r->get_dim()->begin();
  for (auto re = r->get_dim()->end(); ritr != re; ++iitr, ++ritr) {
    if (!Constant().is_static_constant(*iitr)) {
      // ERROR: Is this a non-constant subscript in a net-lval?
      if (net_lval_) {
        error("Found non-constant array subscript in target of continuous assignment", *iitr);
      } 
    }
    else if (Evaluate().get_value(*iitr).to_int() > Evaluate().get_range(*ritr).first) {
      // WARN: Can we say for sure that this value is out of range?
      if (!decl_check_) {
        warn("Array subscript is out of range of declared dimension for this variable", *iitr);
      }
    }
  }    
  return iitr;
}

void TypeCheck::check_arity(const ModuleInstantiation* mi, const Identifier* port, const Expression* arg) {
  // Nothing to do if this is a scalar instantiation
  if (mi->is_null_range()) {
    return;
  }

  // TODO: REMOVE THIS CHECK TO ACTIVATE SUPPORT FOR INSTANTIATION ARRAYS
  return error("Cascade does not currently provide support for instantiation arrays", mi);

  // Ports and arguments with matching widths are okay
  const auto pw = Evaluate().get_width(port);
  const auto aw = Evaluate().get_width(arg);
  if (pw == aw) {
    return;
  }
  // Arguments that divide evenly by number of instances are okay
  const auto mw = Evaluate().get_range(mi->get_range()).first + 1;
  if ((aw % mw == 0) && (aw / mw == pw)) {
    return;
  }

  // Anything else is an error
  error("Found an arity mismatch between module array instantiation and the declared width of one of its ports", mi);
}

} // namespace cascade
