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

#include "src/target/core/de10/boxer.h"

#include <cassert>
#include <sstream>
#include <vector>
#include "src/base/stream/indstream.h"
#include "src/target/core/de10/de10_logic.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/print/text/text_printer.h"

using namespace std;

namespace cascade {

Boxer::Boxer() : Builder() { }

std::string Boxer::box(MId id, const ModuleDeclaration* md, const De10Logic* de) {
  md_ = md;
  de_ = de;

  ModuleInfo info(md_);

  stringstream ss;
  indstream os(ss);

  os << "module M" << id << "(" << endl;
  os.tab();
  os << "input  wire       __clk," << endl;
  os << "input  wire       __read," << endl;
  os << "input  wire[7:0]  __vid," << endl;
  os << "input  wire[31:0] __in," << endl;
  os << "output reg[31:0]  __out," << endl;
  os << "output wire       __wait" << endl;
  os.untab();
  os << ");" << endl;
  os << endl;
  os.tab();

  // Declare the variable table for this module. Contains enough storage for
  // all inputs and stateful variables in this module. Note that (1) stateful
  // elements should NEVER include any inputs and (2) stateful elements may
  // include some, but not necessarily all, outputs.
    
  os << "// Variable Table:" << endl;
  for (auto t = de_->table_begin(), te = de_->table_end(); t != te; ++t) {
    if (!t->second.materialized) {
      continue;
    }
    for (size_t i = 0, ie = t->second.size; i < ie; ++i) {
      os << "reg[31:0] __var_" << (t->second.index+i) << " /* TODO... = <initial value> */;";
      if (de->open_loop_enabled() && (t->first == de->open_loop_clock())) {
        os << " // Open loop clock";
      }
      os << endl;
    }
  }
  os << endl;

  // Declare intermediate state for stateful elements. This is where we will
  // store update values while we wait for the update latch to be set. 

  os << "// Update State" << endl;
  for (auto s : info.stateful()) {
    auto rd = dynamic_cast<RegDeclaration*>(s->get_parent()->clone());
    rd->replace_id(new Identifier(s->get_ids()->front()->get_readable_sid() + "_next"));
    rd->replace_val(new Maybe<Expression>());
    TextPrinter(os) << rd << "\n";
    delete rd;
  }
  os << "reg[" << (de_->table_size()-1) << ":0] __update_mask = 0;" << endl;
  os << "reg[" << (de_->table_size()-1) << ":0] __next_update_mask = 0;" << endl;
  os << endl;

  // Declare intermediate state for systasks. This is where we will store
  // notification flags for tasks while we wait for the user to query for
  // events.

  os << "// Sys Task State" << endl;
  os << "reg[31:0] __task_mask = 0;" << endl;
  os << "reg[31:0] __next_task_mask = 0;" << endl;
  os << endl;

  // Declare control variables. These are used to communicate execution state
  // back and forth with the host. 
  
  os << "// Control State:" << endl;
  os << "reg       __live = 0;" << endl;
  os << "reg[31:0] __open_loop = 0;" << endl;
  os << "reg[31:0] __open_loop_itrs = 0;" << endl;
  os << endl;

  // Define view variables on top of the variable table for inputs and stateful
  // elements. This will allow the program to reference inputs and stateful
  // elements in the variable table without modification.

  os << "// View Variables:" << endl;
  for (auto v = de_->map_begin(), ve = de_->map_end(); v != ve; ++v) {
    const auto titr = de_->table_find(v->first);
    assert(titr != de_->table_end());
    if (!titr->second.materialized) {
      continue;
    }

    Maybe<RangeExpression>* re = nullptr;
    if (auto nd = dynamic_cast<const NetDeclaration*>(v->first->get_parent())) {
      re = nd->get_dim();
    } else if (auto rd = dynamic_cast<const RegDeclaration*>(v->first->get_parent())) {
      re = rd->get_dim();
    } else {
      assert(false);
    }
    os << "wire";
    if (!re->null()) {
      TextPrinter(os) << "[" << re->get()->get_upper() << ":" << re->get()->get_lower() << "]";
    }
    TextPrinter(os) << " " << v->first << " = {";
    for (int i = titr->second.size-1; i >= 0; --i) {
       os << "__var_" << (titr->second.index+i);
       if (i > 0) {
        os << ",";
       }
    }
    os << "};" << endl;
  }
  os << endl;

  // Emit the original program logic. The builder interface for this class is
  // used to make several small modifications to the program text. Declarations
  // for inputs and stateful elements are removed (they were converted to view
  // variables above), port declarations for non-stateful outputs are demoted
  // to normal declarations, and system tasks are replaced by updates to the
  // sys task mask.

  os << "// Original Program Logic:" << endl;
  task_id_ = 0;
  for (auto mi : *md->get_items()) {
    auto temp = mi->accept(this);
    if (temp != nullptr) {
      TextPrinter(os) << temp << "\n";
      delete temp;
    }
  }
  os << endl;

  // Logic for updates. An update is pending whenever an intermediate
  // value differs from its stateful counterpart. Updates are triggered
  // whenever the user forces a read of the update latch or we are in open loop
  // mode.

  os << "// Update Logic:" << endl;
  os << "wire[" << (de_->table_size()-1) << ":0] __update_queue = __update_mask ^ __next_update_mask;" << endl;
  os << "wire __there_are_updates = |__update_queue;" << endl;
  os << "wire __apply_updates = (__read && (__vid == " << de->update_idx() << ")) | (__there_are_updates && (__open_loop > 0));" << endl;
  os << "always @(posedge __clk) begin" << endl;
  os.tab();
  os << "__update_mask <= __apply_updates ? __next_update_mask : __update_mask;" << endl;
  os.untab();
  os << "end" << endl;
  os << endl;

  // Logic for systasks. The task mask is cleared whenever the user forces a
  // read of the mask.

  os << "// Sys Task Logic:" << endl;
  os << "wire[31:0] __task_queue = __task_mask ^ __next_task_mask;" << endl;
  os << "wire __there_were_tasks = |__task_queue;" << endl;
  os << "always @(posedge __clk) begin" << endl;
  os.tab();
  os << "__task_mask <= (__read && (__vid == " << de->sys_task_idx() << ")) ? __next_task_mask : __task_mask;" << endl;
  os.untab();
  os << "end" << endl;
  os << endl;

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

  // Logic for the variable table.  Requesting a read of a specific variable
  // overwrites its value.  Requesting an update forces all stateful variables
  // to latch the values of their counterparts.  In all other cases, variables
  // keep the same value. Special consideration is given to the open loop
  // clock.

  os << "// Variable Table Control Logic:" << endl;
  os << "always @(posedge __clk) begin" << endl;
  os.tab();
  for (auto t = de_->table_begin(), te = de_->table_end(); t != te; ++t) {
    // Ignore variables that weren't materialized or correspond to sys tasks
    if (!t->second.materialized || (Resolve().get_resolution(t->first) != t->first)) {
      continue;
    }
    const auto w = t->second.val.size();
    for (size_t i = 0; i < t->second.size; ++i) {
      stringstream sub;
      const auto upper = min(32*(i+1),w)-1;
      const auto lower = 32*i;
      if (upper == 0) {
        // Do nothing 
      } else if (upper > lower) {
        sub << "[" << upper << ":" << lower << "]";
      } else {
        sub << "[" << lower << "]";
      }
      os << "__var_" << (t->second.index+i) << " <= ";
      if (de->open_loop_enabled() && (t->first == de->open_loop_clock())) {
        TextPrinter(os) << "__open_loop_tick ? {31'b0, ~" << t->first << "} : ";
      } 
      os << "(__read && (__vid == " << (t->second.index+i) << ")) ? __in : ";
      if (info.is_stateful(t->first)) {
        TextPrinter(os) << " (__apply_updates && __update_queue[" << (t->second.index+i) << "]) ? " << t->first << "_next" << sub.str() << " : ";
      }
      TextPrinter(os) << t->first << sub.str() << ";";
      os << endl;
    }
  }
  os.untab();
  os << "end" << endl;
  os << endl;

  // Output logic. Controls which parts of which variables are propagated to
  // out in response to the value of vid.

  os << "// Output Control Logic:" << endl;
  os << "always @(*) begin" << endl;
  os.tab();
  os << "case (__vid)" << endl;
  os.tab();
  os << de_->there_are_updates_idx() << ": __out = __there_are_updates;" << endl;
  os << de_->sys_task_idx() << ": __out = __task_queue;" << endl;
  os << de_->open_loop_idx() << ": __out = __open_loop_itrs;" << endl;
  for (auto t = de->table_begin(), te = de->table_end(); t != te; ++t) {
    if (t->second.materialized) {
      for (size_t i = 0; i < t->second.size; ++i) {
        os << (t->second.index+i) << ": __out = __var_" << (t->second.index+i) << ";" << endl;
      } 
      continue;
    }
    const auto w = t->second.val.size();
    for (size_t i = 0; i < t->second.size; ++i) {
      stringstream sub;
      const auto upper = min(32*(i+1),w)-1;
      const auto lower = 32*i;
      if (upper == 0) {
        // Do nothing 
      } else if (upper > lower) {
        sub << "[" << upper << ":" << lower << "]";
      } else {
        sub << "[" << lower << "]";
      }
      TextPrinter(os) << (t->second.index+i) << ": __out = " << t->first << sub.str() << ";\n";
    }
  }
  os << "default: __out = 0;" << endl;
  os.untab();
  os << "endcase" << endl;
  os.untab();
  os << "end" << endl;
  os << endl;

  os.untab();
  os << "endmodule";

  return ss.str();
}

Attributes* Boxer::build(const Attributes* as) {
  // Quartus doesn't have full support for annotations. At this point, we can just delete them.
  (void) as;
  return new Attributes(new Many<AttrSpec>());
}

ModuleItem* Boxer::build(const RegDeclaration* rd) {
  // Stateful variables have been reified to views and can be ignored.
  // Everything else is downgraded to a regular declaration.
  return ModuleInfo(md_).is_stateful(rd->get_id()) ? nullptr : rd->clone();
}

ModuleItem* Boxer::build(const PortDeclaration* pd) {
  // Stateful variables and inputs have been reified to views and can be ignored.
  // Everything else is downgraded to a regular declaration.
  ModuleInfo info(md_);
  return info.is_stateful(pd->get_decl()->get_id()) || info.is_input(pd->get_decl()->get_id()) ? nullptr : pd->get_decl()->clone();
}

Statement* Boxer::build(const NonblockingAssign* na) {
  // Create empty blocks for true and false branches (we'll never populate the
  // false branch)
  const auto t = new SeqBlock(
    new Maybe<Identifier>(),
    new Many<Declaration>(),
    new Many<Statement>()
  );
  const auto f = t->clone();

  // Look up the target of this assignment and compute its indices in the
  // variable table.
  const auto lhs = na->get_assign()->get_lhs();
  const auto titr = de_->table_find(Resolve().get_resolution(lhs));
  assert(titr != de_->table_end());
  assert(titr->second.materialized);

  Maybe<Expression>* idx = nullptr;
  if (titr->second.size == 1) {
    idx = new Maybe<Expression>(new Number(Bits(32, (uint32_t)titr->second.index)));
  } else {
    idx = new Maybe<Expression>(new RangeExpression(titr->second.index+titr->second.size, titr->second.index));
  }

  // Replace the original assignment with an assignment to a temporary variable
  t->get_stmts()->push_back(new NonblockingAssign(
    na->get_ctrl()->clone(),
    new VariableAssign(
      new Identifier(
        new Many<Id>(new Id(lhs->get_ids()->front()->get_readable_sid() + "_next", new Maybe<Expression>())),
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
        new Many<Id>(new Id("__next_update_mask", new Maybe<Expression>())),
        idx
      ),
      new UnaryExpression(
        UnaryExpression::TILDE,
        new Identifier(
          new Many<Id>(new Id("__next_update_mask", new Maybe<Expression>())),
          idx->clone()
        )
      )
    )
  ));

  // Return a conditional statement in place of the original assignment
  return new ConditionalStatement(new Identifier("__live"), t, f);
}

Statement* Boxer::build(const DisplayStatement* ds) {
  return mangle_systask(task_id_++, ds->get_args());
}

Statement* Boxer::build(const FinishStatement* fs) {
  (void) fs;
  return mangle_systask(task_id_++, nullptr);
}

Statement* Boxer::build(const WriteStatement* ws) {
  return mangle_systask(task_id_++, ws->get_args());
}

Statement* Boxer::mangle_systask(size_t id, const Many<Expression>* args) {
  // Create empty blocks for true and false branches (we'll never populate the
  // false branch)
  const auto t = new SeqBlock(
    new Maybe<Identifier>(),
    new Many<Declaration>(),
    new Many<Statement>()
  );
  const auto f = t->clone();

  // Insert an assignment to the task mask
  t->get_stmts()->push_back(new NonblockingAssign(
    new Maybe<TimingControl>(),
    new VariableAssign(
      new Identifier(
        new Many<Id>(new Id("__next_task_mask", new Maybe<Expression>())),
        new Maybe<Expression>(new Number(Bits(32, (uint32_t)id)))
      ),
      new UnaryExpression(
        UnaryExpression::TILDE,
        new Identifier(
          new Many<Id>(new Id("__next_task_mask", new Maybe<Expression>())),
          new Maybe<Expression>(new Number(Bits(32, (uint32_t)id)))
        )
      )
    )    
  ));

  // Latch the values of any args attached to this function
  if (args != nullptr) {
    for (auto a : *args) {
      if (auto v = dynamic_cast<const Identifier*>(a)) {
        const auto titr = de_->table_find(v);
        assert(titr != de_->table_end());
        const auto w = titr->second.val.size();
       
        for (size_t i = 0; i < titr->second.size; ++i) {
          stringstream ss;
          ss << "__var_" << titr->second.index + i;

          const auto upper = min(32*(i+1),w)-1;
          const auto lower = 32*i;

          t->get_stmts()->push_back(new NonblockingAssign(
            new Maybe<TimingControl>(),
            new VariableAssign(
              new Identifier(ss.str()),
              new Identifier(
                new Many<Id>(v->get_ids()->front()->clone()),
		upper == 0 ? new Maybe<Expression>() :
                upper == lower ? new Maybe<Expression>(new Number(Bits(32, (uint32_t)upper))) :
                  new Maybe<Expression>(new RangeExpression(upper+1, lower))
              )
            )    
          ));
        }
      }
    }
  }

  // Return a conditional statement in place of the original assignment
  return new ConditionalStatement(new Identifier("__live"), t, f);
}

} // namespace cascade
