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

#include <sstream>
#include "target/core/de10/de10_logic.h"
#include "target/core/de10/pass/machinify.h"
#include "target/core/de10/pass/text_mangle.h"
#include "target/core/de10/pass/trigger_reschedule.h"
#include "verilog/analyze/module_info.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"
#include "verilog/print/text/text_printer.h"

using namespace std;

namespace cascade {

string De10Rewrite::run(const ModuleDeclaration* md, const De10Logic* de, size_t slot)  {
  stringstream ss;

  // Generate index tables before doing anything even remotely invasive
  TriggerIndex ti;
  md->accept(&ti);

  // Emit a new declaration. The module name is formed using the slot id
  // assigned by the quartus server.
  ss.str(string());
  ss << "M" << static_cast<int>(slot);
  auto* res = new ModuleDeclaration(
    new Attributes(),
    new Identifier(ss.str())
  );

  emit_port_vars(res);
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

  emit_update_logic(res, de);
  emit_state_logic(res, de, &mfy);
  emit_trigger_logic(res, &ti);
  emit_open_loop_logic(res, de);
  emit_var_logic(res, md, de);
  emit_output_logic(res, md, de);

  // Holy cow! We're done!
  ss.str(string());
  TextPrinter(ss) << res;
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

void De10Rewrite::emit_port_vars(ModuleDeclaration* res) {
  // This is the avalon memory mapped slave interface.

  res->push_back_ports(new ArgAssign(nullptr, new Identifier("__clk")));
  res->push_back_ports(new ArgAssign(nullptr, new Identifier("__read")));
  res->push_back_ports(new ArgAssign(nullptr, new Identifier("__vid")));
  res->push_back_ports(new ArgAssign(nullptr, new Identifier("__in")));
  res->push_back_ports(new ArgAssign(nullptr, new Identifier("__out")));
  res->push_back_ports(new ArgAssign(nullptr, new Identifier("__wait")));

  res->push_back_items(new PortDeclaration(new Attributes(), PortDeclaration::Type::INPUT, new NetDeclaration(
    new Attributes(), new Identifier("__clk"), Declaration::Type::UNSIGNED, nullptr
  )));
  res->push_back_items(new PortDeclaration(new Attributes(), PortDeclaration::Type::INPUT, new NetDeclaration(
    new Attributes(), new Identifier("__read"), Declaration::Type::UNSIGNED, nullptr
  )));
  res->push_back_items(new PortDeclaration(new Attributes(), PortDeclaration::Type::INPUT, new NetDeclaration(
    new Attributes(), new Identifier("__vid"), Declaration::Type::UNSIGNED, new RangeExpression(14, 0)
  )));
  res->push_back_items(new PortDeclaration(new Attributes(), PortDeclaration::Type::INPUT, new NetDeclaration(
    new Attributes(), new Identifier("__in"), Declaration::Type::UNSIGNED, new RangeExpression(32, 0)
  )));
  res->push_back_items(new PortDeclaration(new Attributes(), PortDeclaration::Type::OUTPUT, new RegDeclaration(
    new Attributes(), new Identifier("__out"), Declaration::Type::UNSIGNED, new RangeExpression(32, 0), nullptr
  )));
  res->push_back_items(new PortDeclaration(new Attributes(), PortDeclaration::Type::OUTPUT, new NetDeclaration(
    new Attributes(), new Identifier("__wait"), Declaration::Type::UNSIGNED, nullptr
  )));

  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__read_prev"), Declaration::Type::UNSIGNED, nullptr, new Number(Bits(32, 0))
  ));
  res->push_back_items(new NetDeclaration(
    new Attributes(), new Identifier("__read_request"), Declaration::Type::UNSIGNED
  ));
}

void De10Rewrite::emit_var_table(ModuleDeclaration* res, const De10Logic* de) {
  // This is the hardware image of the table owned by the de logic core.
  // We emit a separate variable for each segment: variables and expressions.

  const auto var_seg_arity = max(static_cast<size_t>(16), de->get_table().var_size());
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier(new Id("__var"), new RangeExpression(var_seg_arity, 0)), Declaration::Type::UNSIGNED, new RangeExpression(32, 0), nullptr
  ));
  const auto expr_seg_arity = max(static_cast<size_t>(16), de->get_table().expr_size());
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier(new Id("__expr"), new RangeExpression(expr_seg_arity, 0)), Declaration::Type::UNSIGNED, new RangeExpression(32, 0), nullptr
  ));
}

void De10Rewrite::emit_shadow_vars(ModuleDeclaration* res, const ModuleDeclaration* md, const De10Logic* de) {
  // These are where update values are stored between calls to evaluate() and
  // update(). These declarations are sorted lexicographically to ensure
  // deterministic code.

  const auto update_arity = max(static_cast<size_t>(32), de->get_table().var_size());

  map<string, RegDeclaration*> shadows;
  for (auto* s : ModuleInfo(md).stateful()) {
    assert(s->get_parent()->is(Node::Tag::reg_declaration));
    auto* rd = static_cast<RegDeclaration*>(s->get_parent()->clone());
    rd->get_id()->purge_ids();
    rd->get_id()->push_back_ids(new Id(s->front_ids()->get_readable_sid() + "_next"));
    rd->replace_val(nullptr);
    shadows.insert(make_pair(rd->get_id()->front_ids()->get_readable_sid(), rd));
  }
  for (auto& s : shadows) {
    res->push_back_items(s.second);
  }
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__prev_update_mask"), Declaration::Type::UNSIGNED, new RangeExpression(update_arity, 0), new Number(Bits(32, 0))
  ));
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__update_mask"), Declaration::Type::UNSIGNED, new RangeExpression(update_arity, 0), new Number(Bits(32, 0))
  ));
}

void De10Rewrite::emit_view_vars(ModuleDeclaration* res, const ModuleDeclaration* md, const De10Logic* de) {
  // These are the variables from the original program which have been
  // relocated into the variable table. These declarations are sorted
  // lexicograhically to ensure deterministic code.

  map<string, pair<NetDeclaration*, vector<ContinuousAssign*>>> views;
  for (auto v = de->get_table().var_begin(), ve = de->get_table().var_end(); v != ve; ++v) {
    // Ignore variables whose state we don't set directly (non-stateful outputs)
    // or things that are only in the variable table because they appear in
    // tasks. 

    ModuleInfo info(md);
    if (!info.is_input(v->first) && !info.is_stateful(v->first)) {
      continue;
    }

    const auto* p = v->first->get_parent();
    assert(p != nullptr);
    assert(p->is_subclass_of(Node::Tag::declaration));
    const auto* decl = static_cast<const Declaration*>(p);
    const auto* re = decl->get_dim();
    const auto type = decl->get_type();
   
    auto* nd = new NetDeclaration(
      new Attributes(), v->first->clone(), type, (re == nullptr) ? nullptr : re->clone()
    ); 
    vector<ContinuousAssign*> cas;
    for (size_t i = 0, ie = v->second.elements; i < ie; ++i) {
      auto* lhs = v->first->clone();
      lhs->purge_dim();
      emit_subscript(lhs, i, ie, Evaluate().get_arity(v->first));
      auto* rhs = new Concatenation();
      for (size_t j = 0, je = v->second.words_per_element; j < je; ++j) {
        rhs->push_back_exprs(new Identifier(new Id("__var"), new Number(Bits(32, v->second.begin + (i+1)*je-j-1))));
      }
      auto* ca = new ContinuousAssign(lhs, rhs);
      cas.push_back(ca);
    }
    views[nd->get_id()->front_ids()->get_readable_sid()] = make_pair(nd, cas);
  }
  for (auto& v : views) {
    res->push_back_items(v.second.first);
    for (auto* ca : v.second.second) {
      res->push_back_items(ca);
    }
  }
}

void De10Rewrite::emit_trigger_vars(ModuleDeclaration* res, const TriggerIndex* ti) {
  // Emit declarations lexicographically to ensure deterministic code
  map<string, const Identifier*> vars;
  for (auto& e : ti->negedges_) {
    vars[e.first] = e.second;
  }
  for (auto& e : ti->posedges_) {
    vars[e.first] = e.second;
  }

  for (auto& v : vars) {
    const auto* decl = static_cast<const Declaration*>(v.second->get_parent());
    const auto* re = decl->get_dim();
    res->push_back_items(new RegDeclaration(
      new Attributes(), new Identifier(v.first+"_prev"), Declaration::Type::UNSIGNED, (re != nullptr) ? re->clone() : nullptr, new Number(Bits(false))
    ));
  }
}

void De10Rewrite::emit_update_logic(ModuleDeclaration* res, const De10Logic* de) {
  // An update is pending whenever a shadow variable's value differs from its
  // counterpart. Updates are triggered whenever the user forces a read of the
  // update latch or we are in open loop mode.

  const auto update_arity = max(static_cast<size_t>(32), de->get_table().var_size());

  res->push_back_items(new NetDeclaration(
    new Attributes(), new Identifier("__update_queue"), Declaration::Type::UNSIGNED, new RangeExpression(update_arity, 0)
  ));
  res->push_back_items(new ContinuousAssign(
    new Identifier("__update_queue"), 
    new BinaryExpression(new Identifier("__prev_update_mask"), BinaryExpression::Op::CARAT, new Identifier("__update_mask"))
  ));
  res->push_back_items(new NetDeclaration(
    new Attributes(), new Identifier("__there_are_updates"), Declaration::Type::UNSIGNED
  ));
  res->push_back_items(new ContinuousAssign(
    new Identifier("__there_are_updates"), 
    new UnaryExpression(UnaryExpression::Op::PIPE, new Identifier("__update_queue"))
  ));
  res->push_back_items(new NetDeclaration(
    new Attributes(), new Identifier("__apply_updates"), Declaration::Type::UNSIGNED
  ));
  res->push_back_items(new ContinuousAssign(
    new Identifier("__apply_updates"), 
    new BinaryExpression(
      new BinaryExpression(
        new Identifier("__read_request"), 
        BinaryExpression::Op::AAMP, 
        new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->get_table().apply_update_index())))
      ),
      BinaryExpression::Op::PPIPE,
      new BinaryExpression(
        new Identifier("__there_are_updates"), 
        BinaryExpression::Op::AAMP,
        new Identifier("__open_loop_tick")
      )
    )
  ));
  res->push_back_items(new NetDeclaration(
    new Attributes(), new Identifier("__drop_updates"), Declaration::Type::UNSIGNED
  ));
  res->push_back_items(new ContinuousAssign(
    new Identifier("__drop_updates"), 
    new BinaryExpression(
      new Identifier("__read_request"), 
      BinaryExpression::Op::AAMP, 
      new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->get_table().drop_update_index())))
    )
  ));
  res->push_back_items(new AlwaysConstruct(new TimingControlStatement(
    new EventControl(new Event(Event::Type::POSEDGE, new Identifier("__clk"))),
    new NonblockingAssign(
      new Identifier("__prev_update_mask"),
      new ConditionalExpression(
        new BinaryExpression(new Identifier("__apply_updates"), BinaryExpression::Op::PPIPE, new Identifier("__drop_updates")),
        new Identifier("__update_mask"), 
        new Identifier("__prev_update_mask")
      )
    ))
  )); 
}

void De10Rewrite::emit_state_logic(ModuleDeclaration* res, const De10Logic* de, const Machinify* mfy) {
  res->push_back_items(new NetDeclaration(
    new Attributes(), new Identifier("__there_were_tasks"), Declaration::Type::UNSIGNED
  ));
  if (mfy->begin() == mfy->end()) {
    res->push_back_items(new ContinuousAssign(
      new Identifier("__there_were_tasks"),
      new Number(Bits(false))
    )); 
  } else {
    auto* c = new Concatenation();
    for (auto& g : *mfy) {
      auto* var = g.name();
      var->push_back_ids(new Id("__task_id"));
      c->push_back_exprs(new BinaryExpression(var, BinaryExpression::Op::BEQ, new Number(Bits(32, 0))));
    }
    res->push_back_items(new ContinuousAssign(
      new Identifier("__there_were_tasks"),
      new UnaryExpression(UnaryExpression::Op::PIPE, c)
    ));
  }
  res->push_back_items(new NetDeclaration(
    new Attributes(), new Identifier("__all_final"), Declaration::Type::UNSIGNED
  ));
  if (mfy->begin() == mfy->end()) {
    res->push_back_items(new ContinuousAssign(
      new Identifier("__all_final"),
      new Number(Bits(true))
    )); 
  } else {
    auto* c = new Concatenation();
    for (auto& g : *mfy) {
      auto* var = g.name();
      var->push_back_ids(new Id("__state"));
      auto* fin = g.name();
      fin->push_back_ids(new Id("__final"));
      c->push_back_exprs(new BinaryExpression(var, BinaryExpression::Op::EEQ, fin));
    }
    res->push_back_items(new ContinuousAssign(
      new Identifier("__all_final"),
      new UnaryExpression(UnaryExpression::Op::AMP, c)
    ));
  }
  res->push_back_items(new NetDeclaration(
    new Attributes(), new Identifier("__continue"), Declaration::Type::UNSIGNED
  ));
  res->push_back_items(new ContinuousAssign(
    new Identifier("__continue"),
    new BinaryExpression(
      new BinaryExpression(
        new Identifier("__read_request"),
        BinaryExpression::Op::AAMP,
        new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->get_table().resume_index())))
      ),
      BinaryExpression::Op::PPIPE,
      new BinaryExpression(
        new UnaryExpression(UnaryExpression::Op::BANG, new Identifier("__all_final")),
        BinaryExpression::Op::AAMP,
        new UnaryExpression(UnaryExpression::Op::BANG, new Identifier("__there_were_tasks"))
      )
    )
  ));
  res->push_back_items(new NetDeclaration(
    new Attributes(), new Identifier("__reset"), Declaration::Type::UNSIGNED
  ));
  res->push_back_items(new ContinuousAssign(
    new Identifier("__reset"),
    new BinaryExpression(
      new Identifier("__read_request"),
      BinaryExpression::Op::AAMP,
      new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->get_table().reset_index())))
    )
  ));
  res->push_back_items(new NetDeclaration(
    new Attributes(), new Identifier("__done"), Declaration::Type::UNSIGNED
  ));
  res->push_back_items(new ContinuousAssign(
    new Identifier("__done"),
    new BinaryExpression(
      new Identifier("__all_final"),
      BinaryExpression::Op::AAMP,
      new UnaryExpression(UnaryExpression::Op::BANG, new Identifier("__there_were_tasks"))
    )
  ));
}

void De10Rewrite::emit_trigger_logic(ModuleDeclaration* res, const TriggerIndex* ti) {
  // We're munging both of these sets together to avoid duplicate assignments. But by
  // doing this, we're losing the deterministic ordering we have thanks to the way 
  // these sets are built. This means we need to sort these assignments by name.
  map<string, const Identifier*> vars;
  for (auto& e : ti->negedges_) {
    vars[e.first] = e.second;
  }
  for (auto& e : ti->posedges_) {
    vars[e.first] = e.second;
  }

  auto* sb = new SeqBlock();
  for (auto& v : vars) {
    sb->push_back_stmts(new NonblockingAssign(
      new Identifier(v.first+"_prev"),
      v.second->clone()
    ));
  }
  res->push_back_items(new AlwaysConstruct(new TimingControlStatement(
    new EventControl(new Event(Event::Type::POSEDGE, new Identifier("__clk"))),
    sb
  ))); 

  // Everything else should be deterministic
  for (auto& e : ti->negedges_) {
    res->push_back_items(new NetDeclaration(
      new Attributes(), new Identifier(e.first+"_negedge"), Declaration::Type::UNSIGNED
    ));
    res->push_back_items(new ContinuousAssign(
      new Identifier(e.first+"_negedge"),
      new BinaryExpression(
        new BinaryExpression(new Identifier(e.first+"_prev"), BinaryExpression::Op::EEQ, new Number(Bits(true))),
        BinaryExpression::Op::AAMP,
        new BinaryExpression(e.second->clone(), BinaryExpression::Op::EEQ, new Number(Bits(false)))
      )  
    ));
  }
  for (auto& e : ti->posedges_) {
    res->push_back_items(new NetDeclaration(
      new Attributes(), new Identifier(e.first+"_posedge"), Declaration::Type::UNSIGNED
    ));
    res->push_back_items(new ContinuousAssign(
      new Identifier(e.first+"_posedge"),
      new BinaryExpression(
        new BinaryExpression(new Identifier(e.first+"_prev"), BinaryExpression::Op::EEQ, new Number(Bits(false))),
        BinaryExpression::Op::AAMP,
        new BinaryExpression(e.second->clone(), BinaryExpression::Op::EEQ, new Number(Bits(true)))
      )  
    ));
  }
  res->push_back_items(new NetDeclaration(
    new Attributes(), new Identifier("__any_triggers"), Declaration::Type::UNSIGNED
  ));
  if (ti->posedges_.empty() && ti->negedges_.empty()) {
    res->push_back_items(new ContinuousAssign(
      new Identifier("__any_triggers"),
      new Number(Bits(32, 0))
    ));
  } else {
    auto* c = new Concatenation();
    for (auto& e : ti->negedges_) {
      c->push_back_exprs(new Identifier(e.first+"_negedge"));
    }
    for (auto& e : ti->posedges_) {
      c->push_back_exprs(new Identifier(e.first+"_posedge"));
    }
    res->push_back_items(new ContinuousAssign(
      new Identifier("__any_triggers"),
      new UnaryExpression(UnaryExpression::Op::PIPE, c)
    ));
  }
}

void De10Rewrite::emit_open_loop_logic(ModuleDeclaration* res, const De10Logic* de) {
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__open_loop"), Declaration::Type::UNSIGNED, new RangeExpression(32, 0), new Number(Bits(32, 0))
  ));
  res->push_back_items(new AlwaysConstruct(new TimingControlStatement(
    new EventControl(new Event(Event::Type::POSEDGE, new Identifier("__clk"))),
    new NonblockingAssign(
      new Identifier("__open_loop"),
      new ConditionalExpression(
        new BinaryExpression(
          new Identifier("__read_request"),
          BinaryExpression::Op::AAMP,
          new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->get_table().open_loop_index())))
        ),
        new Identifier("__in"),
        new ConditionalExpression(
          new Identifier("__open_loop_tick"),
          new BinaryExpression(new Identifier("__open_loop"), BinaryExpression::Op::MINUS, new Number(Bits(true))),
          new Identifier("__open_loop")
        )
      )
    )  
  )));

  res->push_back_items(new NetDeclaration(
    new Attributes(), new Identifier("__open_loop_tick"), Declaration::Type::UNSIGNED
  ));
  res->push_back_items(new ContinuousAssign(
    new Identifier("__open_loop_tick"), 
    new BinaryExpression(
      new Identifier("__all_final"), 
      BinaryExpression::Op::AAMP,
      new BinaryExpression(
        new UnaryExpression(UnaryExpression::Op::BANG, new Identifier("__any_triggers")),
        BinaryExpression::Op::AAMP,
        new BinaryExpression(new Identifier("__open_loop"), BinaryExpression::Op::GT, new Number(Bits(32, 0)))
      )
    )
  ));
}

void De10Rewrite::emit_var_logic(ModuleDeclaration* res, const ModuleDeclaration* md, const De10Logic* de) {
  // Requesting a read of a specific variable overwrites its value.  Requesting
  // an update forces all stateful variables to latch their shadow values. This
  // logic is sorted lexicographically to guarantee deterministic code.

  map<size_t, NonblockingAssign*> logic;
  for (auto t = de->get_table().var_begin(), te = de->get_table().var_end(); t != te; ++t) {
    // If this variable hasn't been reified into the variable table we don't
    // need to emit any update logic.
    ModuleInfo info(md);
    if (!info.is_input(t->first) && !info.is_stateful(t->first)) {
      continue;
    }

    const auto arity = Evaluate().get_arity(t->first);
    const auto w = t->second.bits_per_element;

    size_t idx = t->second.begin;
    for (size_t i = 0, ie = t->second.elements; i < ie; ++i) {
      for (size_t j = 0, je = t->second.words_per_element; j < je; ++j) {
        auto* lhs = new Identifier(new Id("__var"), new Number(Bits(32, idx)));

        Expression* rhs = lhs->clone();
        if (info.is_stateful(t->first)) {
          auto* id = new Identifier(t->first->front_ids()->get_readable_sid() + "_next");
          emit_subscript(id, i, ie, arity);
          emit_slice(id, w, j);
          rhs = new ConditionalExpression(
            new BinaryExpression(
              new Identifier("__apply_updates"), 
              BinaryExpression::Op::AAMP, 
              new Identifier(new Id("__update_queue"), new Number(Bits(32, idx)))),
            id,
            rhs
          );
        } 
        rhs = new ConditionalExpression(
          new BinaryExpression(
            new Identifier("__read_request"),
            BinaryExpression::Op::AAMP,
            new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, idx)))
          ),
          new Identifier("__in"),
          rhs
        );
        if (de->open_loop_enabled() && (t->first == de->open_loop_clock())) {
          auto* concat = new Concatenation();
          concat->push_back_exprs(new Number(Bits(31, 0), Number::Format::BIN));
          concat->push_back_exprs(new UnaryExpression(UnaryExpression::Op::TILDE, t->first->clone()));
          rhs = new ConditionalExpression(
            new Identifier("__open_loop_tick"),
            concat,
            rhs
          );
        }

        logic[idx] = new NonblockingAssign(lhs, rhs);
        ++idx;
      }
    }
  }
  for (auto t = de->get_table().expr_begin(), te = de->get_table().expr_end(); t != te; ++t) {
    assert(t->second.elements == 1);
    assert(t->second.words_per_element == 1);
    
    auto* lhs = new Identifier(new Id("__expr"), new Number(Bits(32, t->second.begin)));
    auto* rhs = new ConditionalExpression(
      new BinaryExpression(
        new Identifier("__read_request"),
        BinaryExpression::Op::AAMP,
        new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->get_table().var_size() + t->second.begin)))
      ),
      new Identifier("__in"),
      lhs->clone()
    );
    logic[de->get_table().var_size() + t->second.begin] = new NonblockingAssign(lhs, rhs);
  }

  auto* lsb = new SeqBlock();
  for (auto& l : logic) {
    lsb->push_back_stmts(l.second);
  }
  res->push_back_items(new AlwaysConstruct(new TimingControlStatement(
    new EventControl(new Event(Event::Type::POSEDGE, new Identifier("__clk"))),
    lsb
  ))); 
}

void De10Rewrite::emit_output_logic(ModuleDeclaration* res, const ModuleDeclaration* md, const De10Logic* de) {
  // Assignments are sorted lexicographically to guarantee deterministic code.

  map<size_t, CaseItem*> outputs;
  for (auto t = de->get_table().var_begin(), te = de->get_table().var_end(); t != te; ++t) {
    ModuleInfo info(md);      
    if (info.is_input(t->first) || info.is_stateful(t->first)) {
      continue;
    }
    assert(t->second.elements == 1);
    const auto w = t->second.bits_per_element;
    for (size_t i = 0; i < t->second.words_per_element; ++i) {
      auto* id = t->first->clone();
      id->purge_dim();
      emit_slice(id, w, i);
      auto* ci = new CaseItem(
        new Number(Bits(32, t->second.begin+i)),
        new BlockingAssign(
          new Identifier("__out"),
          id
        )
      );
      outputs[t->second.begin+i] = ci;
    }
  }
  auto* cs = new CaseStatement(CaseStatement::Type::CASE, new Identifier("__vid"));
  cs->push_back_items(new CaseItem(
    new Number(Bits(32, de->get_table().there_are_updates_index())),
    new BlockingAssign(new Identifier("__out"), new Identifier("__there_are_updates"))
  ));
  cs->push_back_items(new CaseItem(
    new Number(Bits(32, de->get_table().there_were_tasks_index())),
    new BlockingAssign(new Identifier("__out"), new Identifier("__machine_0.__task_id"))
  ));
  cs->push_back_items(new CaseItem(
    new Number(Bits(32, de->get_table().done_index())),
    new BlockingAssign(new Identifier("__out"), new Identifier("__done"))
  ));
  cs->push_back_items(new CaseItem(
    new Number(Bits(32, de->get_table().open_loop_index())),
    new BlockingAssign(new Identifier("__out"), new Identifier("__open_loop"))
  ));
  cs->push_back_items(new CaseItem(
    new Number(Bits(32, de->get_table().debug_index())),
    new BlockingAssign(new Identifier("__out"), new Identifier("__machine_0.__state"))
  ));
  for (auto& o : outputs) {
    cs->push_back_items(o.second);
  }
  cs->push_back_items(new CaseItem(new BlockingAssign(
    new Identifier("__out"), 
    new ConditionalExpression(
      new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::LT, new Number(Bits(32, de->get_table().var_size()))),
      new Identifier(new Id("__var"), new Identifier("__vid")),
      new Identifier(new Id("__expr"), new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::MINUS, new Number(Bits(32, de->get_table().var_size()))))
    )
  )));
  res->push_back_items(new AlwaysConstruct(new TimingControlStatement(new EventControl(), cs))); 

  res->push_back_items(new AlwaysConstruct(new TimingControlStatement(
    new EventControl(new Event(Event::Type::POSEDGE, new Identifier("__clk"))),
    new NonblockingAssign(new Identifier("__read_prev"), new Identifier("__read"))
    )));
  res->push_back_items(new ContinuousAssign(
    new Identifier("__read_request"), 
    new BinaryExpression(
      new UnaryExpression(UnaryExpression::Op::BANG, new Identifier("__read_prev")),
      BinaryExpression::Op::AAMP,
      new Identifier("__read")
    )
  ));
  res->push_back_items(new ContinuousAssign(
    new Identifier("__wait"),
    new BinaryExpression(
      new Identifier("__open_loop_tick"),
      BinaryExpression::Op::PPIPE,
      new BinaryExpression(
        new Identifier("__any_triggers"),
        BinaryExpression::Op::PPIPE,
        new Identifier("__continue")
      )
    )
  ));
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
