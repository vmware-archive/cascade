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

#include "src/target/core/de10/de10_rewrite.h"

#include <sstream>
#include "src/target/core/de10/de10_logic.h"
#include "src/target/core/de10/pass/finish_mangle.h"
#include "src/target/core/de10/pass/machinify.h"
#include "src/target/core/de10/pass/text_mangle.h"
#include "src/target/core/de10/pass/trigger_reschedule.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/print/text/text_printer.h"

using namespace std;

namespace cascade {

string De10Rewrite::run(const ModuleDeclaration* md, const De10Logic* de, QuartusServer::Id id)  {
  stringstream ss;

  // Generate index tables before doing anything even remotely invasive
  TriggerIndex ti;
  md->accept(&ti);

  // Emit a new declaration. The module name is formed using the slot id
  // assigned by the quartus server.
  ss.str(string());
  ss << "M" << static_cast<int>(id);
  auto* res = new ModuleDeclaration(
    new Attributes(),
    new Identifier(ss.str())
  );

  emit_port_vars(res);
  emit_var_table(res, de);
  emit_shadow_vars(res, md, de);
  emit_mask_vars(res);
  emit_control_vars(res);
  emit_view_vars(res, de);
  emit_trigger_vars(res, &ti);
  emit_state_vars(res);

  // Emit original program logic
  TextMangle tm(md, de);
  md->accept_items(&tm, res->back_inserter_items());
  Machinify mfy;
  res->accept(&mfy);
  FinishMangle fm(&tm);
  res->accept(&fm);
  TriggerReschedule tr;
  res->accept(&tr);

  emit_update_logic(res, de);
  emit_task_logic(res, de);
  emit_control_logic(res, de);
  emit_var_logic(res, md, de);
  emit_trigger_logic(res, &ti);
  emit_state_logic(res, de);
  emit_output_logic(res, de);

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
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__clk"), false, nullptr
  )));
  res->push_back_items(new PortDeclaration(new Attributes(), PortDeclaration::Type::INPUT, new NetDeclaration(
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__read"), false, nullptr
  )));
  res->push_back_items(new PortDeclaration(new Attributes(), PortDeclaration::Type::INPUT, new NetDeclaration(
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__vid"), false, new RangeExpression(14, 0)
  )));
  res->push_back_items(new PortDeclaration(new Attributes(), PortDeclaration::Type::INPUT, new NetDeclaration(
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__in"), false, new RangeExpression(32, 0)
  )));
  res->push_back_items(new PortDeclaration(new Attributes(), PortDeclaration::Type::OUTPUT, new RegDeclaration(
    new Attributes(), new Identifier("__out"), false, new RangeExpression(32, 0), nullptr
  )));
  res->push_back_items(new PortDeclaration(new Attributes(), PortDeclaration::Type::OUTPUT, new NetDeclaration(
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__wait"), false, nullptr
  )));
}

void De10Rewrite::emit_var_table(ModuleDeclaration* res, const De10Logic* de) {
  // This is the hardware image of the table owned by the de logic core.

  const auto table_dim = max(static_cast<size_t>(32), de->table_size());

  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier(new Id("__var"), new RangeExpression(table_dim, 0)), false, new RangeExpression(32, 0), nullptr
  ));
}

void De10Rewrite::emit_shadow_vars(ModuleDeclaration* res, const ModuleDeclaration* md, const De10Logic* de) {
  // These are where update values are stored between calls to evaluate() and
  // update(). These declarations sorted lexicographically to ensure
  // deterministic code..

  const auto table_dim = max(static_cast<size_t>(32), de->table_size());

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
    new Attributes(), new Identifier("__update_mask"), false, new RangeExpression(table_dim, 0), new Number(Bits(32, 0))
  ));
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__next_update_mask"), false, new RangeExpression(table_dim, 0), new Number(Bits(32, 0))
  ));
}

void De10Rewrite::emit_mask_vars(ModuleDeclaration* res) {
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__task_mask"), false, new RangeExpression(32, 0), new Number(Bits(32, 0))
  ));
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__next_task_mask"), false, new RangeExpression(32, 0), new Number(Bits(32, 0))
  ));
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__io_mask"), false, new RangeExpression(32, 0), new Number(Bits(32, 0))
  ));
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__next_io_mask"), false, new RangeExpression(32, 0), new Number(Bits(32, 0))
  ));
}

void De10Rewrite::emit_control_vars(ModuleDeclaration* res) {
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__live"), false, nullptr, new Number(Bits(32, 0))
  ));
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__open_loop"), false, new RangeExpression(32, 0), new Number(Bits(32, 0))
  ));
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__open_loop_itrs"), false, new RangeExpression(32, 0), new Number(Bits(32, 0))
  ));
}

void De10Rewrite::emit_view_vars(ModuleDeclaration* res, const De10Logic* de) {
  // These are the variables from the original program which have been
  // relocated into the variable table. These declarations are sorted
  // lexicograhically to ensure deterministic code.

  map<string, pair<NetDeclaration*, vector<ContinuousAssign*>>> views;
  for (auto v = de->map_begin(), ve = de->map_end(); v != ve; ++v) {
    const auto titr = de->table_find(v->first);
    assert(titr != de->table_end());
    if (!titr->second.materialized()) {
      continue;
    }
    const auto& vinfo = titr->second;

    const RangeExpression* re = nullptr;
    auto is_signed = false;
    if (vinfo.id()->get_parent()->is(Node::Tag::net_declaration)) {
      auto* nd = static_cast<const NetDeclaration*>(vinfo.id()->get_parent());
      re = nd->get_dim();
      is_signed = nd->get_signed();
    } else if (vinfo.id()->get_parent()->is(Node::Tag::reg_declaration)) {
      auto* rd = static_cast<const RegDeclaration*>(vinfo.id()->get_parent());
      re = rd->get_dim();
      is_signed = rd->get_signed();
    } 
   
    auto* nd = new NetDeclaration(
      new Attributes(), NetDeclaration::Type::WIRE, nullptr, vinfo.id()->clone(), is_signed, (re == nullptr) ? nullptr : re->clone()
    ); 
    vector<ContinuousAssign*> cas;
    for (size_t i = 0, ie = vinfo.elements(); i < ie; ++i) {
      auto* lhs = vinfo.id()->clone();
      lhs->purge_dim();
      emit_subscript(lhs, i, ie, vinfo.arity());
      auto* rhs = new Concatenation();
      for (size_t j = 0, je = vinfo.element_size(); j < je; ++j) {
        rhs->push_back_exprs(new Identifier(new Id("__var"), new Number(Bits(32, vinfo.index() + (i+1)*je-j-1))));
      }
      auto* ca = new ContinuousAssign(new VariableAssign(lhs, rhs));
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
    const RangeExpression* re = nullptr;
    switch (v.second->get_parent()->get_tag()) {
      case Node::Tag::net_declaration:
        re = static_cast<const NetDeclaration*>(v.second->get_parent())->get_dim();
        break;
      case Node::Tag::reg_declaration:
        re = static_cast<const RegDeclaration*>(v.second->get_parent())->get_dim();
        break;
      default:
        assert(false);
        break;
    }
    res->push_back_items(new RegDeclaration(
      new Attributes(), new Identifier(v.first+"_prev"), false, (re != nullptr) ? re->clone() : nullptr, new Number(Bits(false))
    ));
  }

  for (auto& e : ti->negedges_) {
    res->push_back_items(new NetDeclaration(
      new Attributes(), NetDeclaration::Type::WIRE, new Identifier(e.first+"_negedge"), false
    ));
    res->push_back_items(new ContinuousAssign(new VariableAssign(
      new Identifier(e.first+"_negedge"),
      new BinaryExpression(
        new BinaryExpression(new Identifier(e.first+"_prev"), BinaryExpression::Op::EEQ, new Number(Bits(true))),
        BinaryExpression::Op::AAMP,
        new BinaryExpression(e.second->clone(), BinaryExpression::Op::EEQ, new Number(Bits(false)))
      )  
    )));
  }
  for (auto& e : ti->posedges_) {
    res->push_back_items(new NetDeclaration(
      new Attributes(), NetDeclaration::Type::WIRE, new Identifier(e.first+"_posedge"), false
    ));
    res->push_back_items(new ContinuousAssign(new VariableAssign(
      new Identifier(e.first+"_posedge"),
      new BinaryExpression(
        new BinaryExpression(new Identifier(e.first+"_prev"), BinaryExpression::Op::EEQ, new Number(Bits(false))),
        BinaryExpression::Op::AAMP,
        new BinaryExpression(e.second->clone(), BinaryExpression::Op::EEQ, new Number(Bits(true)))
      )  
    )));
  }
}

void De10Rewrite::emit_state_vars(ModuleDeclaration* res) {
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__resume"), false, nullptr, new Number(Bits(false))
  ));
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__reset"), false, nullptr, new Number(Bits(false))
  ));
  res->push_back_items(new NetDeclaration(
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__done"), false, nullptr
  ));
}

void De10Rewrite::emit_update_logic(ModuleDeclaration* res, const De10Logic* de) {
  // An update is pending whenever a shadow variable's value differs from its
  // counterpart. Updates are triggered whenever the user forces a read of the
  // update latch or we are in open loop mode.

  const auto table_dim = max(static_cast<size_t>(32), de->table_size());

  res->push_back_items(new NetDeclaration(
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__update_queue"), false, new RangeExpression(table_dim, 0)
  ));
  res->push_back_items(new ContinuousAssign(new VariableAssign(
    new Identifier("__update_queue"), 
    new BinaryExpression(new Identifier("__update_mask"), BinaryExpression::Op::CARAT, new Identifier("__next_update_mask"))
  )));
  res->push_back_items(new NetDeclaration(
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__there_are_updates"), false, nullptr
  ));
  res->push_back_items(new ContinuousAssign(new VariableAssign(
    new Identifier("__there_are_updates"), 
    new UnaryExpression(UnaryExpression::Op::PIPE, new Identifier("__update_queue"))
  )));
  res->push_back_items(new NetDeclaration(
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__apply_updates"), false, nullptr
  ));
  res->push_back_items(new ContinuousAssign(new VariableAssign(
    new Identifier("__apply_updates"), 
    new BinaryExpression(
      new BinaryExpression(new Identifier("__read"), BinaryExpression::Op::AAMP, 
        new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->update_idx())))),
      BinaryExpression::Op::PPIPE,
      new BinaryExpression(new Identifier("__there_are_updates"), BinaryExpression::Op::AAMP,
        new BinaryExpression(new Identifier("__open_loop"), BinaryExpression::Op::GT, new Number(Bits(false))))
    )
  )));
  res->push_back_items(new AlwaysConstruct(new TimingControlStatement(
    new EventControl(new Event(Event::Type::POSEDGE, new Identifier("__clk"))),
    new NonblockingAssign(new VariableAssign(
      new Identifier("__update_mask"),
      new ConditionalExpression(new Identifier("__apply_updates"), new Identifier("__next_update_mask"), new Identifier("__update_mask"))
    ))
  ))); 
}

void De10Rewrite::emit_task_logic(ModuleDeclaration* res, const De10Logic* de) {
  // Both masks are cleared whenever the user forces a read of the mask.

  res->push_back_items(new NetDeclaration(
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__task_queue"), false, new RangeExpression(32, 0)
  ));
  res->push_back_items(new ContinuousAssign(new VariableAssign(
    new Identifier("__task_queue"), 
    new BinaryExpression(new Identifier("__task_mask"), BinaryExpression::Op::CARAT, new Identifier("__next_task_mask"))
  )));
  res->push_back_items(new NetDeclaration(
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__there_were_tasks"), false, nullptr
  ));
  res->push_back_items(new ContinuousAssign(new VariableAssign(
    new Identifier("__there_were_tasks"), 
    new UnaryExpression(UnaryExpression::Op::PIPE, new Identifier("__task_queue"))
  )));
  res->push_back_items(new AlwaysConstruct(new TimingControlStatement(
    new EventControl(new Event(Event::Type::POSEDGE, new Identifier("__clk"))),
    new NonblockingAssign(new VariableAssign(
      new Identifier("__task_mask"),
      new ConditionalExpression(
        new BinaryExpression(
          new Identifier("__read"),
          BinaryExpression::Op::AAMP,
          new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->sys_task_idx())))
        ),
        new Identifier("__next_task_mask"), 
        new Identifier("__task_mask"))
    ))
  ))); 
  res->push_back_items(new NetDeclaration(
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__io_queue"), false, new RangeExpression(32, 0)
  ));
  res->push_back_items(new ContinuousAssign(new VariableAssign(
    new Identifier("__io_queue"), 
    new BinaryExpression(new Identifier("__io_mask"), BinaryExpression::Op::CARAT, new Identifier("__next_io_mask"))
  )));
  res->push_back_items(new NetDeclaration(
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__there_was_io"), false, nullptr
  ));
  res->push_back_items(new ContinuousAssign(new VariableAssign(
    new Identifier("__there_was_io"), 
    new UnaryExpression(UnaryExpression::Op::PIPE, new Identifier("__io_queue"))
  )));
  res->push_back_items(new AlwaysConstruct(new TimingControlStatement(
    new EventControl(new Event(Event::Type::POSEDGE, new Identifier("__clk"))),
    new NonblockingAssign(new VariableAssign(
      new Identifier("__io_mask"),
      new ConditionalExpression(
        new BinaryExpression(
          new Identifier("__read"),
          BinaryExpression::Op::AAMP,
          new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->io_task_idx())))
        ),
        new Identifier("__next_io_mask"), 
        new Identifier("__io_mask"))
    ))
  ))); 
}

void De10Rewrite::emit_control_logic(ModuleDeclaration* res, const De10Logic* de) {
  // The live and open loop variables are set in response to user-initiated
  // reads. The open loop iteration counter is reset whenever we go into open
  // loop and ticks whenever we are in open loop and there are no tasks or
  // updates.

  res->push_back_items(new NetDeclaration(
    new Attributes(), NetDeclaration::Type::WIRE, nullptr, new Identifier("__open_loop_tick"), false, nullptr
  ));
  res->push_back_items(new ContinuousAssign(new VariableAssign(
    new Identifier("__open_loop_tick"), 
    new BinaryExpression(
      new BinaryExpression(new Identifier("__open_loop"), BinaryExpression::Op::GT, new Number(Bits(false))),
      BinaryExpression::Op::AAMP,
      new BinaryExpression(
        new UnaryExpression(UnaryExpression::Op::BANG, new Identifier("__there_are_updates")),
        BinaryExpression::Op::AAMP,
        new UnaryExpression(UnaryExpression::Op::BANG, new Identifier("__there_were_tasks"))
      )
    )
  )));
  auto* sb = new SeqBlock();
  sb->push_back_stmts(new NonblockingAssign(new VariableAssign(
    new Identifier("__live"),
    new ConditionalExpression(
      new BinaryExpression(
        new Identifier("__read"),
        BinaryExpression::Op::AAMP,
        new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->live_idx())))
      ),
      new Number(Bits(true)),
      new Identifier("__live")
    )
  )));
  sb->push_back_stmts(new NonblockingAssign(new VariableAssign(
    new Identifier("__open_loop"),
    new ConditionalExpression(
      new BinaryExpression(
        new Identifier("__read"),
        BinaryExpression::Op::AAMP,
        new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->open_loop_idx())))
      ),
      new Identifier("__in"),
      new ConditionalExpression(
        new Identifier("__open_loop_tick"),
        new BinaryExpression(new Identifier("__open_loop"), BinaryExpression::Op::MINUS, new Number(Bits(true))),
        new ConditionalExpression(
          new Identifier("__there_were_tasks"), 
          new Number(Bits(false)), 
          new Identifier("__open_loop")
        )
      )
    )
    )));
  sb->push_back_stmts(new NonblockingAssign(new VariableAssign(
    new Identifier("__open_loop_itrs"),
    new ConditionalExpression(
      new BinaryExpression(
        new Identifier("__read"),
        BinaryExpression::Op::AAMP,
        new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->open_loop_idx())))
      ),
      new Number(Bits(false)),
      new ConditionalExpression(
        new Identifier("__open_loop_tick"),
        new BinaryExpression(new Identifier("__open_loop_itrs"), BinaryExpression::Op::PLUS, new Number(Bits(true))),
        new Identifier("__open_loop_itrs")
      )
    )
  )));
  res->push_back_items(new AlwaysConstruct(new TimingControlStatement(
    new EventControl(new Event(Event::Type::POSEDGE, new Identifier("__clk"))),
    sb
  ))); 
  res->push_back_items(new ContinuousAssign(new VariableAssign(
    new Identifier("__wait"),
    new BinaryExpression(new Identifier("__open_loop"), BinaryExpression::Op::GT, new Number(Bits(false)))
  )));
}

void De10Rewrite::emit_var_logic(ModuleDeclaration* res, const ModuleDeclaration* md, const De10Logic* de) {
  // Requesting a read of a specific variable overwrites its value.  Requesting
  // an update forces all stateful variables to latch their shadow values. This
  // logic is sorted lexicographically to guarantee deterministic code.

  map<size_t, NonblockingAssign*> logic;
  for (auto t = de->table_begin(), te = de->table_end(); t != te; ++t) {
    if (!t->second.materialized() || (Resolve().get_resolution(t->first) != t->first)) {
      continue;
    }

    const auto arity = t->second.arity();
    const auto w = t->second.bit_size();

    size_t idx = t->second.index();
    for (size_t i = 0, ie = t->second.elements(); i < ie; ++i) {
      for (size_t j = 0, je = t->second.element_size(); j < je; ++j) {
        auto* lhs = new Identifier(new Id("__var"), new Number(Bits(32, idx)));
        auto* r = t->first->clone();
        r->purge_dim();
        emit_subscript(r, i, ie, arity);
        emit_slice(r, w, j);
        Expression* rhs = r;

        if (ModuleInfo(md).is_stateful(t->first)) {
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
            new Identifier("__read"),
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

        auto* na = new NonblockingAssign(new VariableAssign(lhs, rhs));
        logic[idx] = na;
        ++idx;
      }
    }
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

void De10Rewrite::emit_trigger_logic(ModuleDeclaration* res, const TriggerIndex* ti) {
  // Emit logic lexicographically to ensure deterministic code
  map<string, const Identifier*> vars;
  for (auto& e : ti->negedges_) {
    vars[e.first] = e.second;
  }
  for (auto& e : ti->posedges_) {
    vars[e.first] = e.second;
  }

  auto* sb = new SeqBlock();
  for (auto& v : vars) {
    sb->push_back_stmts(new NonblockingAssign(new VariableAssign(
      new Identifier(v.first+"_prev"),
      v.second->clone()
    )));
  }
  res->push_back_items(new AlwaysConstruct(new TimingControlStatement(
    new EventControl(new Event(Event::Type::POSEDGE, new Identifier("__clk"))),
    sb
  ))); 
}

void De10Rewrite::emit_state_logic(ModuleDeclaration* res, const De10Logic* de) {
  // The resume and reset flags stay high for exactly one clock tick whenever
  // the user forces a read

  auto* sb = new SeqBlock();
  sb->push_back_stmts(new NonblockingAssign(new VariableAssign(
    new Identifier("__resume"),
    new ConditionalExpression(
      new BinaryExpression(
        new Identifier("__read"),
        BinaryExpression::Op::AAMP,
        new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->resume_idx())))
      ),
      new Number(Bits(true)),
      new Number(Bits(false))
    )
  )));
  sb->push_back_stmts(new NonblockingAssign(new VariableAssign(
    new Identifier("__reset"),
    new ConditionalExpression(
      new BinaryExpression(
        new Identifier("__read"),
        BinaryExpression::Op::AAMP,
        new BinaryExpression(new Identifier("__vid"), BinaryExpression::Op::EEQ, new Number(Bits(32, de->reset_idx())))
      ),
      new Number(Bits(true)),
      new Number(Bits(false))
    )
  )));
  res->push_back_items(new AlwaysConstruct(new TimingControlStatement(
    new EventControl(new Event(Event::Type::POSEDGE, new Identifier("__clk"))),
    sb
  ))); 
}

void De10Rewrite::emit_output_logic(ModuleDeclaration* res, const De10Logic* de) {
  // Assignments are sorted lexicographically to guarantee deterministic code.

  map<size_t, CaseItem*> outputs;
  for (auto t = de->table_begin(), te = de->table_end(); t != te; ++t) {
    if (t->second.materialized()) {
      continue;
    }
    assert(t->second.elements() == 1);
    const auto w = t->second.bit_size();
    for (size_t i = 0, ie = t->second.entry_size(); i < ie; ++i) {
      auto* id = t->first->clone();
      id->purge_dim();
      emit_slice(id, w, i);
      auto* ci = new CaseItem(
        new Number(Bits(32, t->second.index()+i)),
        new BlockingAssign(new VariableAssign(
          new Identifier("__out"),
          id
        ))
      );
      outputs[t->second.index()+i] = ci;
    }
  }
  auto* cs = new CaseStatement(CaseStatement::Type::CASE, new Identifier("__vid"));
  cs->push_back_items(new CaseItem(
    new Number(Bits(32, de->there_are_updates_idx())),
    new BlockingAssign(new VariableAssign(new Identifier("__out"), new Identifier("__there_are_updates")))
  ));
  cs->push_back_items(new CaseItem(
    new Number(Bits(32, de->sys_task_idx())),
    new BlockingAssign(new VariableAssign(new Identifier("__out"), new Identifier("__task_queue")))
  ));
  cs->push_back_items(new CaseItem(
    new Number(Bits(32, de->io_task_idx())),
    new BlockingAssign(new VariableAssign(new Identifier("__out"), new Identifier("__io_queue")))
  ));
  cs->push_back_items(new CaseItem(
    new Number(Bits(32, de->done_idx())),
    new BlockingAssign(new VariableAssign(new Identifier("__out"), new Identifier("__done")))
  ));
  cs->push_back_items(new CaseItem(
    new Number(Bits(32, de->open_loop_idx())),
    new BlockingAssign(new VariableAssign(new Identifier("__out"), new Identifier("__open_loop_itrs")))
  ));
  for (auto& o : outputs) {
    cs->push_back_items(o.second);
  }
  cs->push_back_items(new CaseItem(
    new BlockingAssign(new VariableAssign(new Identifier("__out"), new Identifier(new Id("__var"), new Identifier("__vid"))))
  ));
  res->push_back_items(new AlwaysConstruct(new TimingControlStatement(new EventControl(), cs))); 
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
