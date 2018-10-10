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

TypeCheck::TypeCheck(const Program* program) : Loggable(), Visitor() { 
  program_ = program;
  deactivate(false);
  warn_unresolved(true);
  local_only(true);

  outermost_loop_ = nullptr;
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
  clear_logs();
  if (deactivated_) {
    return;
  }

  // RECURSE: Implicit params and ports
  for (auto p : *mi->get_params()) {
    p->get_imp()->accept(this);
  }
  for (auto p : *mi->get_ports()) {
    p->get_imp()->accept(this);
  }

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
  // CHECK: Duplicate or unrecognized port connections or expressions connected to outputs
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
    }
  }
}

void TypeCheck::pre_elaboration_check(const CaseGenerateConstruct* cgc) {
  clear_logs();
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
  clear_logs();
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

  clear_logs();
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
  clear_logs();
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
  Loggable::warn(ss.str());
}

void TypeCheck::error(const string& s, const Node* n) {
  stringstream ss;
  if (n->get_source() == "<top>") {
    TextPrinter(ss) << "In final line of user input:\n";
  } else {
    TextPrinter(ss) << "In " << n->get_source() << " on line " << n->get_line() << ":\n";
  }
  TextPrinter(ss) << s << ": " << n;
  Loggable::error(ss.str());
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
  Loggable::error(ss.str());
}

void TypeCheck::visit(const Identifier* id) {
  // RECURSE: ids and dim
  id->get_ids()->accept(this);
  id->get_dim()->accept(this);
  // EXIT:  Resolution won't work if either of these checks failed.
  if (Loggable::error()) {
    return;
  }

  // CHECK: Reference to undefined identifier
  const auto r = Resolve().get_resolution(id);
  if (r == nullptr) {
    if (warn_unresolved_) {
      warn("Referenece to unresolved identifier", id);
    } else {
      error("Referenece to unresolved identifier", id);
    }
  }
  // CHECK: Little-endian ranges and subscript out of range
  if (!id->get_dim()->null()) {
    const auto rng = Evaluate().get_range(id->get_dim()->get());
    if (rng.first < rng.second) {
      error("No support for little-endian range", id);
    }
    if ((r != nullptr) && (rng.first >= Evaluate().get_width(r))) {
      error("Upper end of range exceeds variable width", id);
    }
  }
  // CHECK: Is this a reference to a genvar outside of a loop generate construct?
  if (outermost_loop_ == nullptr) {
    if ((r != nullptr) && (dynamic_cast<const GenvarDeclaration*>(r->get_parent()) != nullptr)) {
      error("Illegal reference to genvar outside of a loop generate construct", id);
    }
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

void TypeCheck::check_width(const Maybe<RangeExpression>* re) {
  if (re->null()) {
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

} // namespace cascade
