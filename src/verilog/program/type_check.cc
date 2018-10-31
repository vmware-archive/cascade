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
#include "src/verilog/print/text/text_printer.h"
#include "src/verilog/program/elaborate.h"
#include "src/verilog/program/program.h"

using namespace std;

namespace cascade {

TypeCheck::TypeCheck(const Program* program, Log* log) : Visitor() { 
  program_ = program;
  log_ = log;

  deactivate(false);
  warn_unresolved(true);
  local_only(true);

  outermost_loop_ = nullptr;
  exists_bad_id_ = false;
}

void TypeCheck::deactivate(bool val) {
  deactivated_ = val;
}

void TypeCheck::warn_unresolved(bool val) {
  warn_unresolved_ = val;
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
    p->get_imp()->accept(this);
  }
  for (auto p : *mi->get_ports()) {
    p->get_imp()->accept(this);
  }
  // EXIT: Arity checking will fail without access to resolvable ports
  if (exists_bad_id_) {
    exists_bad_id_ = backup;
    return;
  }

  // CHECK: Array properties
  // TODO: Remove this error when support for instance arrays is complete
  if (!mi->get_range()->null()) {
    return error("Cascade does not currently support the use of instance arrays", mi);
  }
  check_width(mi->get_range());

  // CHECK: Duplicate definition for instantiations other than the root
  if (!Navigate(mi).lost()) {
    if (auto v = Navigate(mi).find_duplicate_name(mi->get_iid()->get_ids()->back())) {
      multiple_def(mi->get_iid(), v->get_parent());
    }
    if (Navigate(mi).find_child(mi->get_iid()->get_ids()->back())) {
      error("A nested scope scope with this name already exists in this scope", mi);
    }
  }
  // CHECK: TODO Recursive instantiation

  // CHECK: Undefined type
  const auto itr = program_->decl_find(mi->get_mid());
  if (itr == program_->decl_end()) {
    if (warn_unresolved_) {
      warn("Instantiation refers to unknown type and may fail during elaboration", mi);
    } else {
      error("Instantiation refers to unknown type", mi);
    }
    // EXIT: Can't continue checking without access to source
    return;
  }
  const auto src = itr->second;

  // CHECK: More overrides than declared parameters
  if (mi->get_params()->size() > ModuleInfo(src).ordered_params().size()) {
    error("Instantiation uses too many parameter overrides", mi);
  }
  // CHECK: Duplicate or unrecognized parameters
  if (mi->uses_named_params()) {
    const auto& np = ModuleInfo(src).named_params();
    unordered_set<const Identifier*, HashId, EqId> params;
    for (auto p : *mi->get_params()) {
      if (!params.insert(p->get_exp()->get()).second) {
        error("Instantiation uses duplicate named params", mi);
      }
      if (np.find(p->get_exp()->get()) == np.end()) {
        error("Reference to unresolved parameter", mi);
      }
    }
  }

  // CHECK: More connections than declared ports
  if (mi->get_ports()->size() > ModuleInfo(src).ordered_ports().size()) {
    error("Instantiation uses too many connections", mi);
  }
  // CHECK: Duplicate or unrecognized port connections 
  // CHECK: expressions connected to outputs
  // CHECK: Arity mismatch for instance arrays
  if (mi->uses_named_ports()) {
    const auto& np = ModuleInfo(src).named_ports();
    unordered_set<const Identifier*, HashId, EqId> ports;
    for (auto p : *mi->get_ports()) {
      if (!ports.insert(p->get_exp()->get()).second) {
        error("Instantiation uses duplicate named connections", mi);
      }
      const auto itr = np.find(p->get_exp()->get());
      if (itr == np.end()) {
        error("Reference to unresolved explicit port", mi);
        continue;
      }
      if (p->get_imp()->null()) {
        continue;
      }
      if (ModuleInfo(src).is_output(*itr)) {
        if (dynamic_cast<const Identifier*>(p->get_imp()->get()) == nullptr) {
          error("Cannot connect expression to named output port", mi);
        }
      }
      check_arity(mi, *itr, p->get_imp()->get());
    }
  } 
  if (!mi->uses_named_ports()) {
    const auto& op = ModuleInfo(src).ordered_ports();
    for (size_t i = 0, ie = min(op.size(), mi->get_ports()->size()); i < ie; ++i) {
      const auto p = mi->get_ports()->get(i);
      if (p->get_imp()->null()) {
        continue;
      }
      if (ModuleInfo(src).is_output(op[i])) {
        if (dynamic_cast<const Identifier*>(p->get_imp()->get()) == nullptr) {
          error("Cannot connect expression to ordered output port", mi);
        }
      }
      check_arity(mi, op[i], p->get_imp()->get());
    }
  }
}

void TypeCheck::pre_elaboration_check(const CaseGenerateConstruct* cgc) {
  if (deactivated_) {
    return;
  }
  // CHECK: Constant guard expression 
  if (!Constant().is_constant(cgc->get_cond())) {
    error("Case generate construct requires constant expression as guard", cgc->get_cond());
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
    if (!Constant().is_constant(c->get_if())) {
      error("If generate construct requires constant expression as guard", c->get_if());
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
    if (!Constant().is_constant_genvar(lgc->get_cond())) {
      error("Loop generate construct requires constant expression as guard", lgc->get_cond());
    }
    // CHECK: Initialization and update refer to same variable
    const auto r1 = Resolve().get_resolution(lgc->get_init()->get_lhs());
    const auto r2 = Resolve().get_resolution(lgc->get_update()->get_lhs());
    if (r1 != r2) {
      error("Initialization and update refer to different variables", lgc->get_update()->get_lhs());
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
  if (n->get_source() == "<top>") {
    TextPrinter(ss) << "In final line of user input:\n";
  } else {
    TextPrinter(ss) << "In " << n->get_source() << " on line " << n->get_line() << ":\n";
  }
  TextPrinter(ss) << s << ": " << n;
  log_->warn(ss.str());
}

void TypeCheck::error(const string& s, const Node* n) {
  stringstream ss;
  if (n->get_source() == "<top>") {
    TextPrinter(ss) << "In final line of user input:\n";
  } else {
    TextPrinter(ss) << "In " << n->get_source() << " on line " << n->get_line() << ":\n";
  }
  TextPrinter(ss) << s << ": " << n;
  log_->error(ss.str());
}

void TypeCheck::multiple_def(const Node* n, const Node* m) {
  stringstream ss;
  if (n->get_source() == "<top>") {
    TextPrinter(ss) << "In final line of user input:\n";
  } else {
    TextPrinter(ss) << "In " << n->get_source() << " on line " << n->get_line() << ":\n";
  }
  TextPrinter(ss) << "A variable named " << n << " already appears in this scope.\n";
  TextPrinter(ss) << "Previous declaration appears in ";
  if (m->get_source() == "<top>") {
    TextPrinter(ss) << "previous user input.";      
  } else {
    TextPrinter(ss) << m->get_source() << " on line " << m->get_line() << ".";
  }
  log_->error(ss.str());
}

void TypeCheck::visit(const Identifier* id) {
  // RECURSE: ids and dim
  auto backup = exists_bad_id_;
  id->get_ids()->accept(this);
  id->get_dim()->accept(this);
  // EXIT: Resolution will fail if there's an unresolved id below here
  if (exists_bad_id_) {
    exists_bad_id_ = backup;
    return;
  }

  // CHECK: Reference to undefined identifier
  const auto r = Resolve().get_resolution(id);
  if (r == nullptr) {
    exists_bad_id_ = true;
    if (warn_unresolved_) {
      warn("Reference to unresolved identifier", id);
    } else {
      error("Reference to unresolved identifier", id);
    }
  }
  // CHECK: Is this a reference to a genvar outside of a loop generate construct?
  if (outermost_loop_ == nullptr) {
    if ((r != nullptr) && (dynamic_cast<const GenvarDeclaration*>(r->get_parent()) != nullptr)) {
      error("Illegal reference to genvar outside of a loop generate construct", id);
    }
  }
  // EXIT: Can't continue checking if id can't be resolved
  if (r == nullptr) {
    return;
  }

  // CHECK: Are subscripts valid, if provided?
  const auto cdr = check_deref(r, id);
  // CHECK: Little-endian ranges and subscript out of range
  if (cdr != id->get_dim()->end()) {
    const auto rng = Evaluate().get_range(*cdr);
    if (Evaluate().get_width(r) == 1) {
      error("Slice operator provided for scalar value", id);
    } else if (rng.first < rng.second) {
      error("No support for little-endian range", id);
    } else if ((r != nullptr) && (rng.first >= Evaluate().get_width(r))) {
      error("Upper end of range exceeds variable width", id);
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
    if (!p->get_exp()->null()) {
      error("No support for explicit ports in module declarations", p);
    }
    if (p->get_imp()->null()) {
      error("Missing implicit port", md);
    }
    auto imp = dynamic_cast<const Identifier*>(p->get_imp()->get());
    if (imp == nullptr) {
      error("No support for implicit ports which are not identifiers", p);
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
  ca->get_ctrl()->accept(this);
  ca->get_assign()->accept(this);

  const auto o = Resolve().get_origin(ca->get_assign()->get_lhs());
  if (o != nullptr && ModuleInfo(o).is_stateful(ca->get_assign()->get_lhs())) {
    error("Stateful elements cannot appear on left hand side of blocking assign statements", ca);
  }
}

void TypeCheck::visit(const GenvarDeclaration* gd) {
  // CHECK: Duplicate definition
  if (auto v = Navigate(gd).find_duplicate_name(gd->get_id()->get_ids()->back())) {
    multiple_def(gd->get_id(), v->get_parent());
  }
  if (Navigate(gd).find_child(gd->get_id()->get_ids()->back())) {
    error("A nested scope scope with this name already exists in this scope", gd);
  }
}

void TypeCheck::visit(const IntegerDeclaration* id) {
  // RECURSE: Check for unsupported language features in initial value
  id->get_val()->accept(this);

  // CHECK: Duplicate definition
  if (auto v = Navigate(id).find_duplicate_name(id->get_id()->get_ids()->back())) {
    multiple_def(id->get_id(), v->get_parent());
  }
  if (Navigate(id).find_child(id->get_id()->get_ids()->back())) {
    error("A nested scope scope with this name already exists in this scope", id);
  }
  // CHECK: Integer initialized to constant value
  if (!id->get_val()->null() && !Constant().is_constant(id->get_val()->get())) {
    error("Integer initialization requires constant value", id);
  }
}

void TypeCheck::visit(const LocalparamDeclaration* ld) {
  // RECURSE: Check for unsupported language features in initial value
  ld->get_val()->accept(this);

  // CHECK: Duplicate definition
  if (auto v = Navigate(ld).find_duplicate_name(ld->get_id()->get_ids()->back())) {
    multiple_def(ld->get_id(), v->get_parent());
  }
  if (Navigate(ld).find_child(ld->get_id()->get_ids()->back())) {
    error("A nested scope scope with this name already exists in this scope", ld);
  }
  // CHECK: Width properties
  check_width(ld->get_dim());
}

void TypeCheck::visit(const NetDeclaration* nd) {
  // CHECK: Duplicate definition
  if (auto v = Navigate(nd).find_duplicate_name(nd->get_id()->get_ids()->back())) {
    multiple_def(nd->get_id(), v->get_parent());
  }
  if (Navigate(nd).find_child(nd->get_id()->get_ids()->back())) {
    error("A nested scope scope with this name already exists in this scope", nd);
  }
  // CHECK: Width properties
  check_width(nd->get_dim());
  // CHECK: Delay control statements
  if (!nd->get_ctrl()->null()) {
    error("No support for delay control statements in net declarations", nd);
  }
}

void TypeCheck::visit(const ParameterDeclaration* pd) {
  // RECURSE: Check for unsupported language features in initial value
  pd->get_val()->accept(this);

  // CHECK: Duplicate definition
  if (auto v = Navigate(pd).find_duplicate_name(pd->get_id()->get_ids()->back())) {
    multiple_def(pd->get_id(), v->get_parent());
  }
  if (Navigate(pd).find_child(pd->get_id()->get_ids()->back())) {
    error("A nested scope scope with this name already exists in this scope", pd);
  }
  // CHECK: Width properties
  check_width(pd->get_dim());
}

void TypeCheck::visit(const RegDeclaration* rd) {
  // RECURSE: Check for unsupported language features in initial value
  rd->get_val()->accept(this);

  // CHECK: Duplicate definition
  if (auto v = Navigate(rd).find_duplicate_name(rd->get_id()->get_ids()->back())) {
    multiple_def(rd->get_id(), v->get_parent());
  }
  if (Navigate(rd).find_child(rd->get_id()->get_ids()->back())) {
    error("A nested scope scope with this name already exists in this scope", rd);
  }
  // CHECK: Width properties
  check_width(rd->get_dim());
  // CHECK: Registers initialized to constant value
  if (!rd->get_val()->null() && !Constant().is_constant(rd->get_val()->get())) {
    error("Register initialization requires constant value", rd);
  }
}

void TypeCheck::visit(const ModuleInstantiation* mi) {
  if (local_only_) {
    return;
  }
  assert(Elaborate().is_elaborated(mi));
  auto inst = Elaborate(program_).get_elaboration(mi);
  inst->accept(this);
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

void TypeCheck::check_width(const Maybe<RangeExpression>* re) {
  if (re->null()) {
    return;
  }

  // RECURSE: First check the contents of this expression
  const auto backup = exists_bad_id_;
  re->accept(this);
  // EXIT: Evaluation will fail if there's an unresolved id below here
  if (exists_bad_id_) {
    exists_bad_id_ = backup;
    return;
  }

  const auto rng = Evaluate().get_range(re->get());
  if (rng.first <= rng.second) {
    error("No support for declarations where msb is not strictly greater than lsb", re);
  }
  if (rng.second != 0) {
    error("No support for declarations where lsb is not equal to zero", re);
  }
}

Many<Expression>::const_iterator TypeCheck::check_deref(const Identifier* r, const Identifier* i) {
  const int diff = i->get_dim()->size() - r->get_dim()->size();

  // We need to have at least as many subscripts on i as r is declared with
  if (diff < 0) {
    error("Not enough subscripts provided on array dereference", i);
    return i->get_dim()->end();
  }
  // If we have the same number, the last subscript on i can't be a range
  // There's also nothing to do if the subscripts are both empty
  else if (diff == 0) {
    if (i->get_dim()->empty()) {
      return i->get_dim()->end();
    } else if (dynamic_cast<const RangeExpression*>(i->get_dim()->back())) {
      error("Illegal range expression found where scalar subscript was expected", i);
      return i->get_dim()->end();
    }
  }
  // And if i has more than one too many, that's an error as well
  else if (diff > 1) {
    error("Dereference arity does not match delared arity", i);
    return i->get_dim()->end();
  } 
  
  // Iterate over subscripts
  auto iitr = i->get_dim()->begin();
  auto ritr = r->get_dim()->begin();
  for (auto re = r->get_dim()->end(); ritr != re; ++iitr, ++ritr) {
    // If this is a net declaration, we have to check that subscripts are
    // constant values which are within range of the declared bounds.
    if (dynamic_cast<NetDeclaration*>(r->get_parent())) {
      if (!Constant().is_constant_genvar(*iitr)) {
        error("Non-constant array subscripts in net dereference", i);
        return i->get_dim()->end();
      }
      if (Evaluate().get_value(*iitr).to_int() > Evaluate().get_range(*ritr).first) {
        error("Array subscript out of bounds in net dereference", i);
        return i->get_dim()->end();
      }
    }
  }    
  return iitr;
}

void TypeCheck::check_arity(const ModuleInstantiation* mi, const Identifier* port, const Expression* arg) {
  // Nothing to do if this is a scalar instantiation
  if (mi->get_range()->null()) {
    return;
  }

  // Ports and arguments with matching widths are okay
  const auto pw = Evaluate().get_width(port);
  const auto aw = Evaluate().get_width(arg);
  if (pw == aw) {
    return;
  }
  // Arguments that divide evenly by number of instances are okay
  const auto mw = Evaluate().get_range(mi->get_range()->get()).first + 1;
  if ((aw % mw == 0) && (aw / mw == pw)) {
    return;
  }

  // Anything else is an error
  error("Arity mismatch between array instantiation and argument", mi);
}

} // namespace cascade
