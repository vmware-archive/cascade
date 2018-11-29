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

#include "src/target/core/de10/module_boxer.h"

#include <cassert>
#include <sstream>
#include <vector>
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/print/text/text_printer.h"

using namespace std;

namespace cascade {

std::string ModuleBoxer::box(MId id, const ModuleDeclaration* md, const De10Logic* de) {
  md_ = md;
  de_ = de;

  ModuleInfo info(md_);
  stringstream ss;
  indstream os(ss);

  os << "module M" << id << "(" << endl;
  os.tab();
  os << "input  wire       __clk," << endl;
  os << "input  wire       __read," << endl;
  os << "input  wire[13:0] __vid," << endl;
  os << "input  wire[31:0] __in," << endl;
  os << "output reg[31:0]  __out," << endl;
  os << "output wire       __wait" << endl;
  os.untab();
  os << ");" << endl;
  os << endl;
  os.tab();

  emit_variable_table(os);
  emit_update_state(os, info);
  emit_sys_task_state(os);
  emit_control_state(os);
  emit_view_variables(os);
  emit_program_logic(os);
  emit_update_logic(os);
  emit_sys_task_logic(os);
  emit_control_logic(os);
  emit_variable_table_logic(os, info);
  emit_output_logic(os);

  return ss.str();
}

Attributes* ModuleBoxer::build(const Attributes* as) {
  // Quartus doesn't have full support for annotations. At this point, we can just delete them.
  (void) as;
  return new Attributes(new Many<AttrSpec>());
}

ModuleItem* ModuleBoxer::build(const InitialConstruct* ic) {
  // If we're seeing a non-ignored initial block here, it's a problem.  These
  // should have been handled in software.
  const auto ign = ic->get_attrs()->get<String>("__ignore"); 
  (void) ign;
  assert(ign != nullptr);
  assert(ign->eq("true"));
 
  // Delete this code
  return nullptr; 
}

ModuleItem* ModuleBoxer::build(const RegDeclaration* rd) {
  // Stateful variables have been reified to views and can be ignored.
  // Everything else is downgraded to a regular declaration.
  return ModuleInfo(md_).is_stateful(rd->get_id()) ? nullptr : rd->clone();
}

ModuleItem* ModuleBoxer::build(const PortDeclaration* pd) {
  // Stateful variables and inputs have been reified to views and can be ignored.
  // Everything else is downgraded to a regular declaration.
  ModuleInfo info(md_);
  return info.is_stateful(pd->get_decl()->get_id()) || info.is_input(pd->get_decl()->get_id()) ? nullptr : pd->get_decl()->clone();
}

Statement* ModuleBoxer::build(const NonblockingAssign* na) {
  // Create empty blocks for true and false branches (we'll never populate the
  // false branch)
  const auto t = new SeqBlock(
    new Maybe<Identifier>(),
    new Many<Declaration>(),
    new Many<Statement>()
  );
  const auto f = t->clone();

  // Look up the target of this assignment and the indices it spans in the
  // variable table
  const auto lhs = na->get_assign()->get_lhs();
  const auto r = Resolve().get_resolution(lhs);
  assert(r != nullptr);
  auto idx = new Many<Expression>(get_table_range(r, lhs));

  // Replace the original assignment with an assignment to a temporary variable
  t->get_stmts()->push_back(new NonblockingAssign(
    na->get_ctrl()->clone(),
    new VariableAssign(
      new Identifier(
        new Many<Id>(new Id(lhs->get_ids()->front()->get_readable_sid() + "_next", nullptr)),
        lhs->get_dim()->clone()
      ),
      na->get_assign()->get_rhs()->clone()
    )
  ));
  // Insert a new assignment to the next mask
  t->get_stmts()->push_back(new NonblockingAssign(
    new Maybe<TimingControl>(),
    new VariableAssign(
      new Identifier(
        new Many<Id>(new Id("__next_update_mask", nullptr)),
        idx
      ),
      new UnaryExpression(
        UnaryExpression::TILDE,
        new Identifier(
          new Many<Id>(new Id("__next_update_mask", nullptr)),
          idx->clone()
        )
      )
    )
  ));

  // Return a conditional statement in place of the original assignment
  return new ConditionalStatement(new Identifier("__live"), t, f);
}

Statement* ModuleBoxer::build(const DisplayStatement* ds) {
  return Mangler(de_).mangle(task_id_++, ds->get_args());
}

Statement* ModuleBoxer::build(const FinishStatement* fs) {
  return Mangler(de_).mangle(task_id_++, fs->get_arg());
}

Statement* ModuleBoxer::build(const WriteStatement* ws) {
  return Mangler(de_).mangle(task_id_++, ws->get_args());
}

Expression* ModuleBoxer::get_table_range(const Identifier* r, const Identifier *i) {
  // Look up r in the variable table
  const auto titr = de_->table_find(r);
  assert(titr != de_->table_end());
  assert(titr->second.materialized());

  // Start with an expression for where this variable begins in the variable table
  Expression* idx = new Number(Bits(32, titr->second.index()));

  // Now iterate over the arity of r and compute a symbolic expression 
  auto mul = titr->second.elements();
  auto iitr = i->get_dim()->begin();
  for (auto a : titr->second.arity()) {
    mul /= a;
    idx = new BinaryExpression(
      idx,
      BinaryExpression::PLUS,
      new BinaryExpression(
        (*iitr++)->clone(),
        BinaryExpression::TIMES,
        new Number(Bits(32, mul*titr->second.element_size()))
      )
    );
  }
  return new RangeExpression(idx, RangeExpression::PLUS, new Number(Bits(32, titr->second.element_size())));
}

ModuleBoxer::Mangler::Mangler(const De10Logic* de) : Visitor() {
  de_ = de;
  t_ = nullptr;
}

Statement* ModuleBoxer::Mangler::mangle(size_t id, const Node* args) {
  // Create blocks for true and false (we won't populate the false branch)
  t_ = new SeqBlock(
    new Maybe<Identifier>(),
    new Many<Declaration>(),
    new Many<Statement>()
  );
  const auto f = t_->clone();
  // Insert an assignment to the task mask for this task id
  t_->get_stmts()->push_back(new NonblockingAssign(
    new Maybe<TimingControl>(),
    new VariableAssign(
      new Identifier(
        new Many<Id>(new Id("__next_task_mask", nullptr)),
        new Many<Expression>(new Number(Bits(32, (uint32_t)id)))
      ),
      new UnaryExpression(
        UnaryExpression::TILDE,
        new Identifier(
          new Many<Id>(new Id("__next_task_mask", nullptr)),
          new Many<Expression>(new Number(Bits(32, (uint32_t)id)))
        )
      )
    )    
  ));
  // Descend on args and latch the values of any identifiers in this list
  args->accept(this);
  // Return a conditional statement in place of the original assignment
  return new ConditionalStatement(new Identifier("__live"), t_, f);
}

void ModuleBoxer::Mangler::visit(const Identifier* id) {
  const auto titr = de_->table_find(id);
  assert(titr != de_->table_end());

  // This is a bit nasty. The amount of space we set aside for this argument in
  // the variable table may exceed its actual bit-width. This is because the
  // width of the argument may have been implicitly extended if it's part of an
  // expression. 
  const auto r = Resolve().get_resolution(id);
  assert(r != nullptr);
  const auto w = Evaluate().get_width(r);

  for (size_t i = 0; i < titr->second.entry_size(); ++i) {
    stringstream ss;
    ss << "__var[" << (titr->second.index() + i) << "]";

    const auto upper = min(32*(i+1),w)-1;
    const auto lower = 32*i;

    // Create a sign extension mask: all zeros for unsigned values, 32 copies
    // of id's highest order bit for signed values.
    Expression* sext = nullptr;
    if (Evaluate().get_signed(id)) {
      sext = new MultipleConcatenation(
        new Number("32"),
        new Concatenation(new Many<Expression>(
          new Identifier(
            new Many<Id>(id->get_ids()->front()->clone()),
            w == 1 ?
              new Many<Expression>() :
              new Many<Expression>(new Number(Bits(32, w-1)))
          )
        ))
      );
    } else {
      sext = new Number(Bits(32, 0), Number::HEX);
    }

    // Concatenate the rhs with the sign extension bits
    auto dim = id->get_dim()->clone();
    if (dim->size() > r->get_dim()->size()) {
      dim->purge_to(r->get_dim()->size());
    }
    if (upper == lower) {
      dim->push_back(new Number(Bits(32, upper)));
    } else if (upper > lower) {
      dim->push_back(new RangeExpression(upper+1, lower));
    } 
    auto rhs = new Concatenation(new Many<Expression>(sext));
    rhs->get_exprs()->push_back(new Identifier(
      new Many<Id>(id->get_ids()->front()->clone()), 
      dim
    ));

    // Attach the concatenation to an assignment, we'll always have enough bits now
    t_->get_stmts()->push_back(new NonblockingAssign(
      new Maybe<TimingControl>(),
      new VariableAssign(
        new Identifier(ss.str()),
        rhs
      )
    ));
  }
}

void ModuleBoxer::emit_variable_table(indstream& os) {
  // Declare the variable table for this module. Contains enough storage for
  // all inputs and stateful variables in this module. Note that (1) stateful
  // elements should NEVER include any inputs and (2) stateful elements may
  // include some, but not necessarily all, outputs.

  // For programs with arrays (particularly multi-dimensional arrays) this
  // can produce some very large output if we name every element of the table
  // individually. The only reason we might consider doing this is if we don't
  // want to allocate space for unmaterialized values. But these are generally
  // so few in number that this isn't a huge deal.
    
  os << "// Variable Table:" << endl;
  os << "reg[31:0] __var[" << de_->open_loop_idx() << ":0];" << endl;
  os << endl;
}

void ModuleBoxer::emit_update_state(indstream& os, ModuleInfo& info) {
  // Declare intermediate state for stateful elements. This is where we will
  // store update values while we wait for the update latch to be set. 

  os << "// Update State" << endl;
  for (auto s : info.stateful()) {
    auto rd = dynamic_cast<RegDeclaration*>(s->get_parent()->clone());
    auto ids = new Many<Id>(new Id(s->get_ids()->front()->get_readable_sid() + "_next", nullptr));
    rd->get_id()->replace_ids(ids);
    rd->replace_val(nullptr);
    TextPrinter(os) << rd << "\n";
    delete rd;
  }
  os << "reg[" << ((de_->table_size() < 2) ? 1 : (de_->table_size()-1)) << ":0] __update_mask = 0;" << endl;
  os << "reg[" << ((de_->table_size() < 2) ? 1 : (de_->table_size()-1)) << ":0] __next_update_mask = 0;" << endl;
  os << endl;
}

void ModuleBoxer::emit_sys_task_state(indstream& os) {
  // Declare intermediate state for systasks. This is where we will store
  // notification flags for tasks while we wait for the user to query for
  // events.

  os << "// Sys Task State" << endl;
  os << "reg[31:0] __task_mask = 0;" << endl;
  os << "reg[31:0] __next_task_mask = 0;" << endl;
  os << endl;
}

void ModuleBoxer::emit_control_state(indstream& os) {
  // Declare control variables. These are used to communicate execution state
  // back and forth with the host. 
  
  os << "// Control State:" << endl;
  os << "reg       __live = 0;" << endl;
  os << "reg[31:0] __open_loop = 0;" << endl;
  os << "reg[31:0] __open_loop_itrs = 0;" << endl;
  os << endl;
}

void ModuleBoxer::emit_view_variables(indstream& os) {
  // Define view variables on top of the variable table for inputs and stateful
  // elements. This will allow the program to reference inputs and stateful
  // elements in the variable table without modification.

  os << "// View Variables:" << endl;
  for (auto v = de_->map_begin(), ve = de_->map_end(); v != ve; ++v) {
    const auto titr = de_->table_find(v->first);
    assert(titr != de_->table_end());
    if (!titr->second.materialized()) {
      continue;
    }
    emit_view_decl(os, titr->second);
    emit_view_init(os, titr->second);
  }
  os << endl;
}

void ModuleBoxer::emit_view_decl(indstream& os, const De10Logic::VarInfo& vinfo) {
  RangeExpression* re = nullptr;
  auto is_signed = false;
  if (auto nd = dynamic_cast<const NetDeclaration*>(vinfo.id()->get_parent())) {
    re = nd->get_dim();
    is_signed = nd->get_signed();
  } else if (auto rd = dynamic_cast<const RegDeclaration*>(vinfo.id()->get_parent())) {
    re = rd->get_dim();
    is_signed = rd->get_signed();
  } else {
    assert(false);
  }
  os << "wire";
  if (is_signed) {
    os << " signed";
  }
  if (re != nullptr) {
    TextPrinter(os) << "[" << re->get_upper() << ":0]";
  }
  TextPrinter(os) << " " << vinfo.id() << ";\n";
}

void ModuleBoxer::emit_view_init(indstream& os, const De10Logic::VarInfo& vinfo) {
  const auto arity = vinfo.arity();
  for (size_t i = 0, ie = vinfo.elements(); i < ie; ++i) {
    os << "assign " << vinfo.id()->get_ids()->front()->get_readable_sid();
    emit_subscript(os, i, ie, arity);
    os << " = {";
    for (size_t j = 0, je = vinfo.element_size(); j < je; ++j) {
      if (j > 0) {
        os << ", ";
      }
      os << "__var[" << (vinfo.index() + (i+1)*je - j - 1) << "]";
    }
    os << "};" << endl;
  }
}

void ModuleBoxer::emit_subscript(indstream& os, size_t idx, size_t n, const vector<size_t>& arity) {
  for (auto a : arity) {
    n /= a;
    const auto i = idx / n;
    idx -= i*n;
    os << "[" << i << "]";
  }
}

void ModuleBoxer::emit_program_logic(indstream& os) {
  // Emit the original program logic. The builder interface for this class is
  // used to make several small modifications to the program text. Declarations
  // for inputs and stateful elements are removed (they were converted to view
  // variables above), port declarations for non-stateful outputs are demoted
  // to normal declarations, and system tasks are replaced by updates to the
  // sys task mask.

  os << "// Original Program Logic:" << endl;
  task_id_ = 0;
  for (auto mi : *md_->get_items()) {
    auto temp = mi->accept(this);
    if (temp != nullptr) {
      TextPrinter(os) << temp << "\n";
      delete temp;
    }
  }
  os << endl;
}

void ModuleBoxer::emit_update_logic(indstream& os) {
  // Logic for updates. An update is pending whenever an intermediate
  // value differs from its stateful counterpart. Updates are triggered
  // whenever the user forces a read of the update latch or we are in open loop
  // mode.

  os << "// Update Logic:" << endl;
  os << "wire[" << ((de_->table_size() < 2) ? 1 : (de_->table_size()-1)) << ":0] __update_queue = __update_mask ^ __next_update_mask;" << endl;
  os << "wire __there_are_updates = |__update_queue;" << endl;
  os << "wire __apply_updates = (__read && (__vid == " << de_->update_idx() << ")) | (__there_are_updates && (__open_loop > 0));" << endl;
  os << "always @(posedge __clk) begin" << endl;
  os.tab();
  os << "__update_mask <= __apply_updates ? __next_update_mask : __update_mask;" << endl;
  os.untab();
  os << "end" << endl;
  os << endl;
}

void ModuleBoxer::emit_sys_task_logic(indstream& os) {
  // Logic for systasks. The task mask is cleared whenever the user forces a
  // read of the mask.

  os << "// Sys Task Logic:" << endl;
  os << "wire[31:0] __task_queue = __task_mask ^ __next_task_mask;" << endl;
  os << "wire __there_were_tasks = |__task_queue;" << endl;
  os << "always @(posedge __clk) begin" << endl;
  os.tab();
  os << "__task_mask <= (__read && (__vid == " << de_->sys_task_idx() << ")) ? __next_task_mask : __task_mask;" << endl;
  os.untab();
  os << "end" << endl;
  os << endl;
}

void ModuleBoxer::emit_control_logic(indstream& os) {
  // Logic for control variables. The live and open variables are set in
  // response to user initiated reads.  The open loop iteration counter is
  // reset whenever we go into open loop and ticks whenever we are in open loop
  // and there are no tasks or updates.

  os << "// Control Logic:" << endl;
  os << "wire __open_loop_tick = (__open_loop > 0) && !__there_are_updates && !__there_were_tasks;" << endl;
  os << "always @(posedge __clk) begin" << endl;
  os.tab();
  os << "__live <= (__read && (__vid == " << de_->live_idx() << ")) ? 1'b1 : __live;" << endl;
  os << "__open_loop <= (__read && (__vid == " << de_->open_loop_idx() << ")) ? __in : __open_loop_tick ? (__open_loop - 1) : __there_were_tasks ? 0 : __open_loop;" << endl;
  os << "__open_loop_itrs <= (__read && (__vid == " << de_->open_loop_idx() << ")) ? 0 : __open_loop_tick ? (__open_loop_itrs + 1) : __open_loop_itrs;" << endl;
  os.untab();
  os << "end" << endl;
  os << "assign __wait = __open_loop > 0;" << endl;
  os << endl;
}

void ModuleBoxer::emit_variable_table_logic(indstream& os, ModuleInfo& info) {
  // Logic for the variable table.  Requesting a read of a specific variable
  // overwrites its value.  Requesting an update forces all stateful variables
  // to latch the values of their counterparts.  In all other cases, variables
  // keep the same value. Special consideration is given to the open loop
  // clock.

  os << "// Variable Table Logic:" << endl;
  os << "always @(posedge __clk) begin" << endl;
  os.tab();
  for (auto t = de_->table_begin(), te = de_->table_end(); t != te; ++t) {
    // Ignore variables that weren't materialized or correspond to sys tasks
    if (!t->second.materialized() || (Resolve().get_resolution(t->first) != t->first)) {
      continue;
    }

    const auto arity = t->second.arity();
    const auto w = t->second.bit_size();

    size_t idx = t->second.index();
    for (size_t i = 0, ie = t->second.elements(); i < ie; ++i) {
      for (size_t j = 0, je = t->second.element_size(); j < je; ++j) {

        os << "__var[" << idx << "] <= ";
        if (de_->open_loop_enabled() && (t->first == de_->open_loop_clock())) {
          TextPrinter(os) << "__open_loop_tick ? {31'b0, ~" << t->first << "} : ";
        } 
        os << "(__read && (__vid == " << idx << ")) ? __in : ";
        if (info.is_stateful(t->first)) {
          TextPrinter(os) << "(__apply_updates && __update_queue[" << idx << "]) ? ";
          TextPrinter(os) << t->first->get_ids()->front()->get_readable_sid() << "_next";
          emit_subscript(os, i, ie, arity);
          emit_slice(os, w, j);
          os << " : ";
        }
        TextPrinter(os) << t->first->get_ids()->front();
        emit_subscript(os, i, ie, arity);
        emit_slice(os, w, j);
        os  << ";";
        os << endl;

        ++idx;
      }
    }
  }
  os.untab();
  os << "end" << endl;
  os << endl;
}

void ModuleBoxer::emit_slice(indstream& os, size_t w, size_t i) {
  const auto upper = min(32*(i+1),w)-1;
  const auto lower = 32*i;
  if (upper == 0) {
    // Do nothing 
  } else if (upper > lower) {
    os << "[" << upper << ":" << lower << "]";
  } else {
    os << "[" << lower << "]";
  }
}

void ModuleBoxer::emit_output_logic(indstream& os) {
  // Output logic. Controls which parts of which variables are propagated to
  // out in response to the value of vid.

  os << "// Output Control Logic:" << endl;
  os << "always @(*) begin" << endl;
  os.tab();
  os << "case (__vid)" << endl;
  os.tab();
  // Special cases for control variables first:
  os << de_->there_are_updates_idx() << ": __out = __there_are_updates;" << endl;
  os << de_->sys_task_idx() << ": __out = __task_queue;" << endl;
  os << de_->open_loop_idx() << ": __out = __open_loop_itrs;" << endl;
  // Now emit cases for variables which weren't materialized
  for (auto t = de_->table_begin(), te = de_->table_end(); t != te; ++t) {
    if (!t->second.materialized()) {
      assert(t->second.elements() == 1);
      const auto w = t->second.bit_size();
      for (size_t i = 0, ie = t->second.entry_size(); i < ie; ++i) {
        TextPrinter(os) << (t->second.index()+i) << ": __out = " << t->first;
        emit_slice(os, w, i); 
        os << ";\n";
      }
    }
  }
  // For everything else __vid is an index into the variable table
  os << "default: __out = __var[__vid];" << endl;
  os.untab();
  os << "endcase" << endl;
  os.untab();
  os << "end" << endl;
  os << endl;

  os.untab();
  os << "endmodule";
}

} // namespace cascade
