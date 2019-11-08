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

#ifndef CASCADE_SRC_TARGET_CORE_AVMM_REWRITE_H
#define CASCADE_SRC_TARGET_CORE_AVMM_REWRITE_H

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "target/core/avmm/var_table.h"
#include "target/core/avmm/machinify.h"
#include "target/core/avmm/text_mangle.h"
#include "verilog/analyze/module_info.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/visitors/visitor.h"
#include "verilog/ast/ast.h"
#include "verilog/build/ast_builder.h"
#include "verilog/print/print.h"
#include "verilog/transform/block_flatten.h"

namespace cascade {

template <typename T>
class Rewrite {
  public:
    std::string run(const ModuleDeclaration* md, size_t slot, const VarTable<T>* vt, const Identifier* clock);

  private:
    // Records variables which appear in timing control statements
    struct TriggerIndex : Visitor {
      TriggerIndex();
      ~TriggerIndex() override = default;
      std::map<std::string, const Identifier*> negedges_;
      std::map<std::string, const Identifier*> posedges_;
      void visit(const Event* e) override;
    };

    void emit_avalon_vars(ModuleDeclaration* res);
    void emit_var_table(ModuleDeclaration* res, const VarTable<T>* vt);
    void emit_shadow_vars(ModuleDeclaration* res, const ModuleDeclaration* md, const VarTable<T>* vt);
    void emit_view_vars(ModuleDeclaration* res, const ModuleDeclaration* md, const VarTable<T>* vt);
    void emit_update_vars(ModuleDeclaration* res, const VarTable<T>* vt);
    void emit_state_vars(ModuleDeclaration* res);
    void emit_trigger_vars(ModuleDeclaration* res, const TriggerIndex* ti);
    void emit_open_loop_vars(ModuleDeclaration* res);
    void emit_var_vars(ModuleDeclaration* res, const Machinify* mfy);

    void emit_avalon_logic(ModuleDeclaration* res);
    void emit_update_logic(ModuleDeclaration* res, const VarTable<T>* vt);
    void emit_state_logic(ModuleDeclaration* res, const VarTable<T>* vt, const Machinify* mfy);
    void emit_trigger_logic(ModuleDeclaration* res, const TriggerIndex* ti);
    void emit_open_loop_logic(ModuleDeclaration* res, const VarTable<T>* vt);
    void emit_var_logic(ModuleDeclaration* res, const ModuleDeclaration* md, const VarTable<T>* vt, const Machinify* mfy, const Identifier* open_loop_clock);
    void emit_output_logic(ModuleDeclaration* res, const ModuleDeclaration* md, const VarTable<T>* vt);
          
    void emit_subscript(Identifier* id, size_t idx, size_t n, const std::vector<size_t>& arity) const;
    void emit_slice(Identifier* id, size_t w, size_t i) const;
};

template <typename T>
inline std::string Rewrite<T>::run(const ModuleDeclaration* md, size_t slot, const VarTable<T>* vt, const Identifier* clock) {
  std::stringstream ss;

  // Generate index tables before doing anything even remotely invasive
  TriggerIndex ti;
  md->accept(&ti);

  // Emit a new declaration, with module name based on slot id. This
  // declaration will use the Avalon Memory-mapped slave interface.
  DeclBuilder db;
  db << "module M" << static_cast<int>(slot) << "(__clk, __read, __write, __vid, __in, __out, __wait);" << std::endl;
  db << "input wire __clk;" << std::endl;
  db << "input wire __read;" << std::endl;
  db << "input wire __write;" << std::endl;
  db << "input wire[13:0] __vid;" << std::endl;
  db << "input wire[31:0] __in;" << std::endl;
  db << "output reg[31:0] __out;" << std::endl;
  db << "output wire __wait;" << std::endl;
  db << "endmodule" << std::endl;
  auto *res = db.get();

  emit_avalon_vars(res);
  emit_var_table(res, vt);
  emit_shadow_vars(res, md, vt);
  emit_view_vars(res, md, vt);
  emit_update_vars(res, vt);
  emit_state_vars(res);
  emit_trigger_vars(res, &ti);
  emit_open_loop_vars(res);

  // Emit original program logic
  TextMangle<T> tm(md, vt);
  md->accept_items(&tm, res->back_inserter_items());
  Machinify mfy;
  mfy.run(res);

  emit_var_vars(res, &mfy);
  emit_avalon_logic(res);
  emit_update_logic(res, vt);
  emit_state_logic(res, vt, &mfy);
  emit_trigger_logic(res, &ti);
  emit_open_loop_logic(res, vt);
  emit_var_logic(res, md, vt, &mfy, clock);
  emit_output_logic(res, md, vt);

  // Final cleanup passes
  BlockFlatten().run(res);

  // Holy cow! We're done!
  ss.str(std::string());
  ss << res;
  delete res;
  return ss.str();
}

template <typename T>
inline Rewrite<T>::TriggerIndex::TriggerIndex() : Visitor() { }

template <typename T>
inline void Rewrite<T>::TriggerIndex::visit(const Event* e) {
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

template <typename T>
inline void Rewrite<T>::emit_avalon_vars(ModuleDeclaration* res) {
  ItemBuilder ib;
  ib << "reg __read_prev = 0;" << std::endl;
  ib << "wire __read_request;" << std::endl; 
  ib << "reg __write_prev = 0;" << std::endl;
  ib << "wire __write_request;" << std::endl; 
  res->push_back_items(ib.begin(), ib.end()); 
}

template <typename T>
inline void Rewrite<T>::emit_var_table(ModuleDeclaration* res, const VarTable<T>* vt) {
  ItemBuilder ib;

  // Emit the var table and the expression table
  const auto var_seg_arity = std::max(static_cast<size_t>(16), vt->var_size());
  ib << "reg[31:0] __var[" << (var_seg_arity-1) << ":0];" << std::endl;
  const auto expr_seg_arity = std::max(static_cast<size_t>(16), vt->expr_size());
  ib << "reg[31:0] __expr[" << (expr_seg_arity-1) << ":0];" << std::endl;

  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_shadow_vars(ModuleDeclaration* res, const ModuleDeclaration* md, const VarTable<T>* vt) {
  ModuleInfo info(md);

  // Index the stateful elements in the variable table
  std::map<std::string, typename VarTable<T>::const_var_iterator> vars;
  for (auto v = vt->var_begin(), ve = vt->var_end(); v != ve; ++v) {
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
    ib << rd << std::endl;
    delete rd;
  }

  // Emit update masks for the var table
  const auto update_arity = std::max(static_cast<size_t>(32), vt->var_size());
  ib << "reg[" << (update_arity-1) << ":0] __prev_update_mask = 0;" << std::endl;
  ib << "reg[" << (update_arity-1) << ":0] __update_mask = 0;" << std::endl;

  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_view_vars(ModuleDeclaration* res, const ModuleDeclaration* md, const VarTable<T>* vt) {
  ModuleInfo info(md);

  // Index both inputs and the stateful elements in the variable table
  std::map<std::string, typename VarTable<T>::const_var_iterator> vars;
  for (auto v = vt->var_begin(), ve = vt->var_end(); v != ve; ++v) {
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
    ib << nd << std::endl;
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
      ib << "};" << std::endl;
    }
  }

  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_update_vars(ModuleDeclaration* res, const VarTable<T>* vt) {
  ItemBuilder ib;

  const auto update_arity = std::max(static_cast<size_t>(32), vt->var_size());
  ib << "wire[" << (update_arity-1) << ":0] __update_queue;" << std::endl;
  ib << "wire __there_are_updates;" << std::endl;
  ib << "wire __apply_updates;" << std::endl;
  ib << "wire __drop_updates;" << std::endl;
  
  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_state_vars(ModuleDeclaration* res) {
  ItemBuilder ib;

  ib << "wire __there_were_tasks;" << std::endl;
  ib << "wire __all_final;" << std::endl;
  ib << "wire __continue;" << std::endl;
  ib << "wire __reset;" << std::endl;

  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_trigger_vars(ModuleDeclaration* res, const TriggerIndex* ti) {
  ItemBuilder ib;

  // Index triggers
  std::map<std::string, const Identifier*> vars;
  for (auto& e : ti->negedges_) {
    vars[e.first] = e.second;
  }
  for (auto& e : ti->posedges_) {
    vars[e.first] = e.second;
  }

  // Emit variables for storing previous values of trigger variables
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
    ib << rd << std::endl;
    delete rd; 
  }

  // Emit edge variables (these should be sorted determinstically by virtue of
  // how these sets were built)
  for (const auto& e : ti->negedges_) {
    ib << "wire " << e.first << "_negedge;" << std::endl;
  }
  for (auto& e : ti->posedges_) {
    ib << "wire " << e.first << "_posedge;" << std::endl;
  }
  
  // Emit var for tracking whether any triggers just occurred
  ib << "wire __any_triggers;" << std::endl;

  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_open_loop_vars(ModuleDeclaration* res) {
  ItemBuilder ib;

  ib << "reg[31:0] __open_loop = 0;" << std::endl;
  ib << "wire __open_loop_tick;" << std::endl;

  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_var_vars(ModuleDeclaration* res, const Machinify* mfy) {
  ItemBuilder ib;
  ib << "reg[31:0] __task_id[" << (mfy->end()-mfy->begin()-1) << ":0];" << std::endl;
  ib << "reg[31:0] __state[" << (mfy->end()-mfy->begin()-1) << ":0];" << std::endl;
  ib << "reg __kick[" << (mfy->end()-mfy->begin()-1) << ":0];" << std::endl;
  
  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_avalon_logic(ModuleDeclaration* res) {
  ItemBuilder ib;
  ib << "always @(posedge __clk) __read_prev <= __read;" << std::endl;
  ib << "assign __read_request = (!__read_prev && __read);" << std::endl;
  ib << "always @(posedge __clk) __write_prev <= __write;" << std::endl;
  ib << "assign __write_request = (!__write_prev && __write);" << std::endl;
  res->push_back_items(ib.begin(), ib.end()); 
}

template <typename T>
inline void Rewrite<T>::emit_update_logic(ModuleDeclaration* res, const VarTable<T>* vt) {
  ItemBuilder ib;

  const auto update_arity = std::max(static_cast<size_t>(32), vt->var_size());
  ib << "assign __update_queue = (__prev_update_mask ^ __update_mask);" << std::endl;
  ib << "assign __there_are_updates = |__update_queue;" << std::endl;
  ib << "assign __apply_updates = ((__read_request && (__vid == " << vt->apply_update_index() << ")) || (__there_are_updates && __open_loop_tick));" << std::endl;
  ib << "assign __drop_updates = (__read_request && (__vid == " << vt->drop_update_index() << "));" << std::endl;
  ib << "always @(posedge __clk) __prev_update_mask <= ((__apply_updates || __drop_updates) ? __update_mask : __prev_update_mask);" << std::endl;
  
  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_state_logic(ModuleDeclaration* res, const VarTable<T>* vt, const Machinify* mfy) {
  ItemBuilder ib;

  if (mfy->begin() == mfy->end()) {
    ib << "assign __there_were_tasks = 0;" << std::endl;
    ib << "assign __all_final = 1;" << std::endl;
  } else {
    ib << "assign __there_were_tasks = |{";
    for (auto i = mfy->begin(), ie = mfy->end(); i != ie;) {
      ib << "__task_id[" << i->name() << "] != 0";
      if (++i != ie) {
        ib << ",";
      }
    }
    ib << "};" << std::endl;
    ib << "assign __all_final = &{";
    for (auto i = mfy->begin(), ie = mfy->end(); i != ie; ) {
      ib << "__state[" << i->name() << "] == " << i->final_state(); 
      if (++i != ie) {
        ib << ",";
      }
    }
    ib << "};" << std::endl;
  }

  ib << "assign __continue = ((__read_request && (__vid == " << vt->resume_index() << ")) || (!__all_final && !__there_were_tasks));" << std::endl;
  ib << "assign __reset = (__read_request && (__vid == " << vt->reset_index() << "));" << std::endl;

  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_trigger_logic(ModuleDeclaration* res, const TriggerIndex* ti) {
  ItemBuilder ib;

  // Index trigger variables
  std::set<std::string> vars;
  for (const auto& e : ti->negedges_) {
    vars.insert(e.first);
  }
  for (const auto& e : ti->posedges_) {
    vars.insert(e.first);
  }

  // Emit updates for trigger variables
  ib << "always @(posedge __clk) begin" << std::endl;
  for (const auto& v : vars) {
    ib << v << "_prev <= " << v << ";" << std::endl;
  }
  ib << "end" << std::endl;

  // Emit edge variables (these should be sorted determinstically by virtue of
  // how these sets were built)
  for (const auto& e : ti->negedges_) {
    ib << "assign " << e.first << "_negedge = (" << e.first << "_prev == 1) && (" << e.first << " == 0);" << std::endl;
  }
  for (auto& e : ti->posedges_) {
    ib << "assign " << e.first << "_posedge = (" << e.first << "_prev == 0) && (" << e.first << " == 1);" << std::endl;
  }
  
  // Emit logic for tracking whether any triggers just occurred
  if (ti->posedges_.empty() && ti->negedges_.empty()) {
    ib << "assign __any_triggers = 0;" << std::endl;
  } else {
    ib << "assign __any_triggers = |{";
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
    ib << "};" << std::endl;
  }

  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_open_loop_logic(ModuleDeclaration* res, const VarTable<T>* vt) {
  ItemBuilder ib;

  ib << "always @(posedge __clk) __open_loop <= ((__read_request && (__vid == " << vt->open_loop_index() << ")) ? __in : (__open_loop_tick ? (__open_loop - 1) : __open_loop));" << std::endl;
  ib << "assign __open_loop_tick = (__all_final && (!__any_triggers && (__open_loop > 0)));" << std::endl;

  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_var_logic(ModuleDeclaration* res, const ModuleDeclaration* md, const VarTable<T>* vt, const Machinify* mfy, const Identifier* clock) {
  ModuleInfo info(md);

  // Index both inputs and the stateful elements in the variable table as well
  // as the elements in the expr table.
  std::map<size_t, typename VarTable<T>::const_var_iterator> vars;
  for (auto t = vt->var_begin(), te = vt->var_end(); t != te; ++t) {
    if (info.is_input(t->first) || info.is_stateful(t->first)) {
      vars[t->second.begin] = t;
    }
  }
  std::map<size_t, typename VarTable<T>::const_expr_iterator> exprs;
  for (auto t = vt->expr_begin(), te = vt->expr_end(); t != te; ++t) {
    exprs[vt->var_size() + t->second.begin] = t;
  }

  ItemBuilder ib;
  ib << "always @(posedge __clk) begin" << std::endl;
  for (auto i = mfy->begin(), ie = mfy->end(); i != ie; ++i) {
    ib << i->text() << std::endl;
  }
  for (const auto& v : vars) {
    const auto itr = v.second;
    const auto arity = Evaluate().get_arity(itr->first);
    const auto w = itr->second.bits_per_element;
    auto idx = itr->second.begin;

    for (size_t i = 0, ie = itr->second.elements; i < ie; ++i) {
      for (size_t j = 0, je = itr->second.words_per_element; j < je; ++j) {
        ib << "__var[" << idx << "] <= ";
        if ((clock != nullptr) && (itr->first == clock)) {
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
        ib << "__var[" << idx << "];" << std::endl;
        ++idx;
      }
    }
  }
  for (const auto& e : exprs) {
    const auto itr = e.second;
    assert(itr->second.elements == 1);
    assert(itr->second.words_per_element == 1);
    const auto idx = itr->second.begin;
    ib << "__expr[" << idx << "] <= ((__read_request && (__vid == " << (vt->var_size()+idx) << ")) ? __in : __expr[" << idx << "]);" << std::endl;
  }
  ib << "end" << std::endl;
  
  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_output_logic(ModuleDeclaration* res, const ModuleDeclaration* md, const VarTable<T>* vt) {
  ModuleInfo info(md);      

  // Index the elements in the variable table which aren't inputs or stateful.
  std::map<size_t, typename VarTable<T>::const_var_iterator> outputs;
  for (auto t = vt->var_begin(), te = vt->var_end(); t != te; ++t) {
    if (!info.is_input(t->first) && !info.is_stateful(t->first)) {
      outputs[t->second.begin] = t;
    }
  }

  ItemBuilder ib;
  ib << "always @*" << std::endl;
  ib << "case(__vid)" << std::endl;

  for (const auto& o : outputs) {
    const auto itr = o.second;
    assert(itr->second.elements == 1);
    const auto w = itr->second.bits_per_element;
    for (size_t i = 0; i < itr->second.words_per_element; ++i) {
      ib << (itr->second.begin+i) << ": __out = ";

      auto* id = itr->first->clone();
      id->purge_dim();
      emit_slice(id, w, i);
      ib << id << ";" << std::endl;

      delete id;
    }
  }
  
  ib << vt->there_are_updates_index() << ": __out = __there_are_updates;" << std::endl;
  ib << vt->there_were_tasks_index() << ": __out = __task_id[0];" << std::endl;
  ib << vt->open_loop_index() << ": __out = __open_loop;" << std::endl;
  ib << vt->debug_index() << ": __out = __state[0];" << std::endl;
  ib << "default: __out = ((__vid < " << vt->var_size() << ") ? __var[__vid] : __expr[(__vid - " << vt->var_size() << ")]);" << std::endl;
  ib << "endcase" << std::endl;
  ib << "assign __wait = __read_request || __write_request || __open_loop_tick || __any_triggers || __continue;" << std::endl;

  res->push_back_items(ib.begin(), ib.end());
}

template <typename T>
inline void Rewrite<T>::emit_subscript(Identifier* id, size_t idx, size_t n, const std::vector<size_t>& arity) const {
  for (auto a : arity) {
    n /= a;
    const auto i = idx / n;
    idx -= i*n;
    id->push_back_dim(new Number(Bits(32, i)));
  }
}

template <typename T>
inline void Rewrite<T>::emit_slice(Identifier* id, size_t w, size_t i) const {
  const auto upper = std::min(32*(i+1),w);
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

#endif
