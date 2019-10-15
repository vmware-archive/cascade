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

#include "target/core/de10/de10_rewrite.h"

#include <set>
#include <sstream>
#include "target/core/de10/de10_logic.h"
#include "target/core/de10/pass/machinify.h"
#include "target/core/de10/pass/text_mangle.h"
#include "target/core/de10/pass/trigger_reschedule.h"
#include "verilog/analyze/module_info.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"
#include "verilog/build/ast_builder.h"
#include "verilog/print/print.h"
#include "verilog/transform/block_flatten.h"

using namespace std;

namespace cascade {

string De10Rewrite::run(const ModuleDeclaration* md, const De10Logic* de, size_t slot)  {
  stringstream ss;

  // Generate index tables before doing anything even remotely invasive
  TriggerIndex ti;
  md->accept(&ti);

  // Emit a new declaration, with module name based on slot id. This
  // declaration will use the Avalon Memory-mapped slave interface.
  DeclBuilder db;
  db << "module M" << static_cast<int>(slot) << "(__clk, __read, __vid, __in, __out, __wait);" << endl;
  db << "input wire __clk;" << endl;
  db << "input wire __read;" << endl;
  db << "input wire[13:0] __vid;" << endl;
  db << "input wire[31:0] __in;" << endl;
  db << "output reg[31:0] __out;" << endl;
  db << "output wire __wait;" << endl;
  db << "endmodule" << endl;
  auto *res = db.get();

  emit_avalon_vars(res);
  emit_var_table(res, de);
  emit_shadow_vars(res, md, de);
  emit_view_vars(res, md, de);
  emit_trigger_vars(res, &ti);

  // Emit original program logic
  TextMangle tm(md, de);
  md->accept_items(&tm, res->back_inserter_items());
  Machinify mfy;
  res->accept(&mfy);
  TriggerReschedule tr;
  res->accept(&tr);

  emit_avalon_logic(res);
  emit_update_logic(res, de);
  emit_state_logic(res, de, &mfy);
  emit_trigger_logic(res, &ti);
  emit_open_loop_logic(res, de);
  emit_var_logic(res, md, de);
  emit_output_logic(res, md, de);

  // Final cleanup passes
  BlockFlatten().run(res);

  // Holy cow! We're done!
  ss.str(string());
  ss << res;
  delete res;
  return ss.str();
}

De10Rewrite::TriggerIndex::TriggerIndex() : Visitor() { }

void De10Rewrite::TriggerIndex::visit(const Event* e) {
  assert(e->get_expr()->is(Node::Tag::identifier));
  const auto* i = static_cast<const Identifier*>(e->get_expr());
  const auto* r = Resolve().get_resolution(i);
  assert(r != nullptr);

  switch (e->get_type()) {
    case Event::Type::NEGEDGE:
      negedges_[r->front_ids()->get_readable_sid()] = r;
      break;
    case Event::Type::POSEDGE:
      posedges_[r->front_ids()->get_readable_sid()] = r;
      break;
    default:
      // Don't record untyped edges
      break;
  }
}

void De10Rewrite::emit_avalon_vars(ModuleDeclaration* res) {
  ItemBuilder ib;
  ib << "reg __read_prev = 0;" << endl;
  ib << "wire __read_request;" << endl; 
  res->push_back_items(ib.begin(), ib.end()); 
}

void De10Rewrite::emit_var_table(ModuleDeclaration* res, const De10Logic* de) {
  ItemBuilder ib;

  // Emit the var table and the expression table
  const auto var_seg_arity = max(static_cast<size_t>(16), de->get_table().var_size());
  ib << "reg[31:0] __var[" << (var_seg_arity-1) << ":0];" << endl;
  const auto expr_seg_arity = max(static_cast<size_t>(16), de->get_table().expr_size());
  ib << "reg[31:0] __expr[" << (expr_seg_arity-1) << ":0];" << endl;

  res->push_back_items(ib.begin(), ib.end());
}

void De10Rewrite::emit_shadow_vars(ModuleDeclaration* res, const ModuleDeclaration* md, const De10Logic* de) {
  ModuleInfo info(md);

  // Index the stateful elements in the variable table
  map<string, VarTable32::const_var_iterator> vars;
  for (auto v = de->get_table().var_begin(), ve = de->get_table().var_end(); v != ve; ++v) {
    if (info.is_stateful(v->first)) {
      vars.insert(make_pair(v->first->front_ids()->get_readable_sid(), v));
    }
  }

  // Emit a shadow variable for every element with name suffixed by _next.
  ItemBuilder ib;
  for (const auto& v : vars) {
    const auto itr = v.second;
    assert(itr->first->get_parent() != nullptr);
    assert(itr->first->get_parent()->is(Node::Tag::reg_declaration));
    
    auto* rd = static_cast<const RegDeclaration*>(itr->first->get_parent())->clone();
    rd->get_id()->purge_ids();
    rd->get_id()->push_front_ids(new Id(v.first + "_next"));
    rd->replace_val(nullptr);
    ib << rd << endl;
    delete rd;
  }

  // Emit update masks for the var table
  const auto update_arity = max(static_cast<size_t>(32), de->get_table().var_size());
  ib << "reg[" << (update_arity-1) << ":0] __prev_update_mask = 0;" << endl;
  ib << "reg[" << (update_arity-1) << ":0] __update_mask = 0;" << endl;

  res->push_back_items(ib.begin(), ib.end());
}

void De10Rewrite::emit_view_vars(ModuleDeclaration* res, const ModuleDeclaration* md, const De10Logic* de) {
  ModuleInfo info(md);

  // Index both inputs and the stateful elements in the variable table
  map<string, VarTable32::const_var_iterator> vars;
  for (auto v = de->get_table().var_begin(), ve = de->get_table().var_end(); v != ve; ++v) {
    if (info.is_input(v->first) || info.is_stateful(v->first)) {
      vars.insert(make_pair(v->first->front_ids()->get_readable_sid(), v));
    }
  }

  // Emit views for these variables
  ItemBuilder ib;
  for (const auto& v : vars) {
    const auto itr = v.second;
    assert(itr->first->get_parent() != nullptr);
    assert(itr->first->get_parent()->is_subclass_of(Node::Tag::declaration));
    const auto* d = static_cast<const Declaration*>(itr->first->get_parent());

    auto* nd = new NetDeclaration(
      new Attributes(), 
      d->get_id()->clone(),
      d->get_type(),
      d->is_non_null_dim() ? d->clone_dim() : nullptr
    );
    ib << nd << endl;
    delete nd;
    
    for (size_t i = 0, ie = itr->second.elements; i < ie; ++i) {
      auto* lhs = itr->first->clone();
      lhs->purge_dim();
      emit_subscript(lhs, i, ie, Evaluate().get_arity(itr->first));
      ib << "assign " << lhs << " = {";
      delete lhs;
      
      for (size_t j = 0, je = itr->second.words_per_element; j < je; ) {
        ib << "__var[" << (itr->second.begin + (i+1)*je-j-1) << "]";
        if (++j != je) {
          ib << ",";
        }
      }
      ib << "};" << endl;
    }
  }

  res->push_back_items(ib.begin(), ib.end());
}

void De10Rewrite::emit_trigger_vars(ModuleDeclaration* res, const TriggerIndex* ti) {
  // Index triggers
  map<string, const Identifier*> vars;
  for (auto& e : ti->negedges_) {
    vars[e.first] = e.second;
  }
  for (auto& e : ti->posedges_) {
    vars[e.first] = e.second;
  }

  // Emit variables for storing previous values of trigger variables
  ItemBuilder ib;
  for (const auto& v : vars) {
    assert(v.second->get_parent() != nullptr);
    assert(v.second->get_parent()->is_subclass_of(Node::Tag::declaration));
    const auto* d = static_cast<const Declaration*>(v.second->get_parent());

    auto* rd = new RegDeclaration(
      new Attributes(),
      new Identifier(v.first + "_prev"),
      d->get_type(),
      d->is_non_null_dim() ? d->clone_dim() : nullptr,
      nullptr
    );
    ib << rd << endl;
    delete rd; 
  }

  res->push_back_items(ib.begin(), ib.end());
}

void De10Rewrite::emit_avalon_logic(ModuleDeclaration* res) {
  ItemBuilder ib;
  ib << "always @(posedge __clk) __read_prev <= __read;" << endl;
  ib << "assign __read_request = (!__read_prev && __read);" << endl;
  res->push_back_items(ib.begin(), ib.end()); 
}

void De10Rewrite::emit_update_logic(ModuleDeclaration* res, const De10Logic* de) {
  ItemBuilder ib;

  const auto update_arity = max(static_cast<size_t>(32), de->get_table().var_size());
  ib << "wire[" << (update_arity-1) << ":0] __update_queue = (__prev_update_mask ^ __update_mask);" << endl;
  ib << "wire __there_are_updates = |__update_queue;" << endl;
  ib << "wire __apply_updates = ((__read_request && (__vid == " << de->get_table().apply_update_index() << ")) || (__there_are_updates && __open_loop_tick));" << endl;
  ib << "wire __drop_updates = (__read_request && (__vid == " << de->get_table().drop_update_index() << "));" << endl;
  ib << "always @(posedge __clk) __prev_update_mask <= ((__apply_updates || __drop_updates) ? __update_mask : __prev_update_mask);" << endl;
  
  res->push_back_items(ib.begin(), ib.end());
}

void De10Rewrite::emit_state_logic(ModuleDeclaration* res, const De10Logic* de, const Machinify* mfy) {
  ItemBuilder ib;

  if (mfy->begin() == mfy->end()) {
    ib << "wire __there_were_tasks = 0;" << endl;
    ib << "wire __all_final = 1;" << endl;
  } else {
    ib << "wire __there_were_tasks = |{";
    for (auto i = mfy->begin(), ie = mfy->end(); i != ie;) {
      ib << i->name() << ".__task_id != 0";
      if (++i != ie) {
        ib << ",";
      }
    }
    ib << "};" << endl;
    ib << "wire __all_final = &{";
    for (auto i = mfy->begin(), ie = mfy->end(); i != ie; ) {
      ib << i->name() << ".__state == " << i->name() << ".__final";
      if (++i != ie) {
        ib << ",";
      }
    }
    ib << "};" << endl;
  }

  ib << "wire __continue = ((__read_request && (__vid == " << de->get_table().resume_index() << ")) || (!__all_final && !__there_were_tasks));" << endl;
  ib << "wire __reset = (__read_request && (__vid == " << de->get_table().reset_index() << "));" << endl;
  ib << "wire __done = (__all_final && !__there_were_tasks);" << endl;

  res->push_back_items(ib.begin(), ib.end());
}

void De10Rewrite::emit_trigger_logic(ModuleDeclaration* res, const TriggerIndex* ti) {
  ItemBuilder ib;

  // Index trigger variables
  set<string> vars;
  for (const auto& e : ti->negedges_) {
    vars.insert(e.first);
  }
  for (const auto& e : ti->posedges_) {
    vars.insert(e.first);
  }

  // Emit updates for trigger variables
  ib << "always @(posedge __clk) begin" << endl;
  for (const auto& v : vars) {
    ib << v << "_prev <= " << v << ";" << endl;
  }
  ib << "end" << endl;

  // Emit edge variables (these should be sorted determinstically by virtue of
  // how these sets were built)
  for (const auto& e : ti->negedges_) {
    ib << "wire " << e.first << "_negedge = (" << e.first << "_prev == 1) && (" << e.first << " == 0);" << endl;
  }
  for (auto& e : ti->posedges_) {
    ib << "wire " << e.first << "_posedge = (" << e.first << "_prev == 0) && (" << e.first << " == 1);" << endl;
  }
  
  // Emit logic for tracking whether any triggers just occurred
  if (ti->posedges_.empty() && ti->negedges_.empty()) {
    ib << "wire __any_triggers = 0;" << endl;
  } else {
    ib << "wire __any_triggers = |{";
    for (auto i = ti->negedges_.begin(), ie = ti->negedges_.end(); i != ie; ) {
      ib << i->first << "_negedge";
      if ((++i != ie) || !ti->posedges_.empty()) {
        ib << ",";
      }
    }
    for (auto i = ti->posedges_.begin(), ie = ti->posedges_.end(); i != ie; ) {
      ib << i->first << "_posedge";
      if (++i != ie) {
        ib << ",";
      } 
    }
    ib << "};" << endl;
  }

  res->push_back_items(ib.begin(), ib.end());
}

void De10Rewrite::emit_open_loop_logic(ModuleDeclaration* res, const De10Logic* de) {
  ItemBuilder ib;

  ib << "reg[31:0] __open_loop = 0;" << endl;
  ib << "always @(posedge __clk) __open_loop <= ((__read_request && (__vid == " << de->get_table().open_loop_index() << ")) ? __in : (__open_loop_tick ? (__open_loop - 1) : __open_loop));" << endl;
  ib << "wire __open_loop_tick = (__all_final && (!__any_triggers && (__open_loop > 0)));" << endl;

  res->push_back_items(ib.begin(), ib.end());
}

void De10Rewrite::emit_var_logic(ModuleDeclaration* res, const ModuleDeclaration* md, const De10Logic* de) {
  ModuleInfo info(md);

  // Index both inputs and the stateful elements in the variable table as well
  // as the elements in the expr table.
  map<size_t, VarTable32::const_var_iterator> vars;
  for (auto t = de->get_table().var_begin(), te = de->get_table().var_end(); t != te; ++t) {
    if (info.is_input(t->first) || info.is_stateful(t->first)) {
      vars[t->second.begin] = t;
    }
  }
  map<size_t, VarTable32::const_expr_iterator> exprs;
  for (auto t = de->get_table().expr_begin(), te = de->get_table().expr_end(); t != te; ++t) {
    exprs[de->get_table().var_size() + t->second.begin] = t;
  }

  ItemBuilder ib;
  ib << "always @(posedge __clk) begin" << endl;
  for (const auto& v : vars) {
    const auto itr = v.second;
    const auto arity = Evaluate().get_arity(itr->first);
    const auto w = itr->second.bits_per_element;
    auto idx = itr->second.begin;

    for (size_t i = 0, ie = itr->second.elements; i < ie; ++i) {
      for (size_t j = 0, je = itr->second.words_per_element; j < je; ++j) {
        ib << "__var[" << idx << "] = ";
        if (de->open_loop_enabled() && (itr->first == de->open_loop_clock())) {
          ib << "__open_loop_tick ? {31'd0,~" << itr->first->front_ids()->get_readable_sid() << "} : ";
        }
        ib << "(__read_request && (__vid == " << idx << ")) ? __in : ";
        if (info.is_stateful(itr->first)) {
          auto* id = new Identifier(itr->first->front_ids()->get_readable_sid() + "_next");
          emit_subscript(id, i, ie, arity);
          emit_slice(id, w, j);
          ib << "(__apply_updates && __update_queue[" << idx << "]) ? " << id << " : ";
          delete id;
        }
        ib << "__var[" << idx << "];" << endl;
        ++idx;
      }
    }
  }
  for (const auto& e : exprs) {
    const auto itr = e.second;
    assert(itr->second.elements == 1);
    assert(itr->second.words_per_element == 1);
    const auto idx = itr->second.begin;
    ib << "__expr[" << idx << "] <= ((__read_request && (__vid == " << (de->get_table().var_size()+idx) << ")) ? __in : __expr[" << idx << "]);" << endl;
  }
  ib << "end" << endl;
  
  res->push_back_items(ib.begin(), ib.end());
}

void De10Rewrite::emit_output_logic(ModuleDeclaration* res, const ModuleDeclaration* md, const De10Logic* de) {
  ModuleInfo info(md);      

  // Index the elements in the variable table which aren't inputs or stateful.
  map<size_t, VarTable32::const_var_iterator> outputs;
  for (auto t = de->get_table().var_begin(), te = de->get_table().var_end(); t != te; ++t) {
    if (!info.is_input(t->first) && !info.is_stateful(t->first)) {
      outputs[t->second.begin] = t;
    }
  }

  ItemBuilder ib;
  ib << "always @*" << endl;
  ib << "case(__vid)" << endl;

  for (const auto& o : outputs) {
    const auto itr = o.second;
    assert(itr->second.elements == 1);
    const auto w = itr->second.bits_per_element;
    for (size_t i = 0; i < itr->second.words_per_element; ++i) {
      ib << (itr->second.begin+i) << ": __out = ";

      auto* id = itr->first->clone();
      id->purge_dim();
      emit_slice(id, w, i);
      ib << id << ";" << endl;

      delete id;
    }
  }
  
  ib << de->get_table().there_are_updates_index() << ": __out = __there_are_updates;" << endl;
  ib << de->get_table().there_were_tasks_index() << ": __out = __machine_0.__task_id;" << endl;
  ib << de->get_table().done_index() << ": __out = __done;" << endl;
  ib << de->get_table().open_loop_index() << ": __out = __open_loop;" << endl;
  ib << de->get_table().debug_index() << ": __out = __machine_0.__state;" << endl;
  ib << "default: __out = ((__vid < " << de->get_table().var_size() << ") ? __var[__vid] : __expr[(__vid - " << de->get_table().var_size() << ")]);" << endl;
  ib << "endcase" << endl;
  ib << "assign __wait = (__open_loop_tick || (__any_triggers || __continue));" << endl;

  res->push_back_items(ib.begin(), ib.end());
}

void De10Rewrite::emit_subscript(Identifier* id, size_t idx, size_t n, const std::vector<size_t>& arity) const {
  for (auto a : arity) {
    n /= a;
    const auto i = idx / n;
    idx -= i*n;
    id->push_back_dim(new Number(Bits(32, i)));
  }
}

void De10Rewrite::emit_slice(Identifier* id, size_t w, size_t i) const {
  const auto upper = min(32*(i+1),w);
  const auto lower = 32*i;
  if (upper == 1) {
    // Do nothing 
  } else if (upper > lower) {
    id->push_back_dim(new RangeExpression(upper, lower));
  } else {
    id->push_back_dim(new Number(Bits(32, lower)));
  }
}

} // namespace cascade
