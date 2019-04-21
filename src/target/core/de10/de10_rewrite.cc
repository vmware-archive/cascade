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

#include <algorithm>
#include <sstream>
#include "src/target/core/de10/de10_logic.h"
#include "src/verilog/analyze/module_info.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/print/text/text_printer.h"

using namespace std;

namespace cascade {

ModuleDeclaration* De10Rewrite::run(const ModuleDeclaration* md, const De10Logic* de, QuartusServer::Id id)  {
  // Variables we'll use in a few places
  const auto table_dim = max(static_cast<size_t>(32), de->table_size());

  // Emit a new declaration. The module name is formed using the slot id
  // assigned by the quartus server.
  stringstream ss;
  ss << "M" << static_cast<int>(id);
  auto* res = new ModuleDeclaration(
    new Attributes(),
    new Identifier(ss.str())
  );

  // Emit port declarations. This is the avalon memory mapped slave interface.
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

  // Emit the variable table. This is the hardware image of the table owned by
  // the de logic core.
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier(new Id("__var"), new RangeExpression(de->open_loop_idx()+1, 0)), false, new RangeExpression(32, 0), nullptr
  ));

  // Emit shadow declarations for stateful elements. These are where update
  // values are stored between calls to evaluate() and update(). These
  // declarations sorted lexicographically to ensure deterministic code..
  map<string, RegDeclaration*> shadows;
  for (auto* s : ModuleInfo(md).stateful()) {
    assert(s->get_parent()->is(Node::Tag::reg_declaration));
    auto* rd = static_cast<RegDeclaration*>(s->get_parent()->clone());
    rd->get_id()->purge_ids();
    rd->get_id()->push_back_ids(new Id(s->front_ids()->get_readable_sid() + "_next"));
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

  // Emit mask variables for tracking system tasks and io tasks
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

  // Emit control state variables.
  // TODO(eschkufz) Do we need this __live variable anymore?
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__live"), false, nullptr, new Number(Bits(32, 0))
  ));
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__open_loop"), false, new RangeExpression(32, 0), new Number(Bits(32, 0))
  ));
  res->push_back_items(new RegDeclaration(
    new Attributes(), new Identifier("__open_loop_itrs"), false, new RangeExpression(32, 0), new Number(Bits(32, 0))
  ));

  // Emit declarations for view variables. These are the variables from the
  // original program which have been relocated into the variable table. These
  // declarations are sorted lexicograhically to ensure deterministic code.
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
      append_subscript(lhs, i, ie, vinfo.arity());
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

  // Emit original program logic
  TaskMangle tm(de);
  md->accept_items(&tm);
  RewriteText rt(md, de);
  md->accept_items(&rt, res->back_inserter_items());
  Machinify mfy;
  res->accept(&mfy);
  FinishMangle fm(&tm);
  res->accept(&fm);

  // Emit logic for update requests. An update is pending whenever a shadow
  // variable's value differs from its counterpart. Updates are triggered
  // whenever the user forces a read of the update latch or we are in open loop
  // mode.
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

  // Emit logic for handling system tasks. Both masks are cleared whenever the
  // user forces a read of the mask.
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

  // Emit logic for control variables. The live and open loop variables are set
  // in response to user-initiated reads. The open loop iteration counter is
  // reset whenever we go into open loop and ticks whenever we are in open loop
  // and there are no tasks or updates.
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
        new ConditionalExpression(new Identifier("__there_were_tasks"), new Number(Bits(false)), new Identifier("__open_loop"))
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

  // Emit variable table logic. Requesting a read of a specific variable
  // overwrites its value.  Requesting an update forces all stateful variables
  // to latch their shadow values. This logic is sorted lexicographically to
  // guarantee deterministic code.
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
        append_subscript(r, i, ie, arity);
        append_slice(r, w, j);
        Expression* rhs = r;

        if (ModuleInfo(md).is_stateful(t->first)) {
          auto* id = new Identifier(t->first->front_ids()->get_readable_sid() + "_next");
          append_subscript(id, i, ie, arity);
          append_slice(id, w, j);
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

  // Emit output logic. These assignments are sorted lexicographically to
  // ensure deterministic code.
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
      append_slice(id, w, i);
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

  // Holy cow! We're done!
  return res;
}

void De10Rewrite::append_subscript(Identifier* id, size_t idx, size_t n, const std::vector<size_t>& arity) const {
  for (auto a : arity) {
    n /= a;
    const auto i = idx / n;
    idx -= i*n;
    id->push_back_dim(new Number(Bits(32, i)));
  }
}

void De10Rewrite::append_slice(Identifier* id, size_t w, size_t i) const {
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

De10Rewrite::TaskMangle::TaskMangle(const De10Logic* de) : Visitor() {
  de_ = de;
  within_task_ = false;
  io_idx_ = 0;
  task_idx_ = 0;
}

Node* De10Rewrite::TaskMangle::get(const Node* n) {
  stringstream ss;
  TextPrinter(ss) << n;

  const auto itr = tasks_.find(ss.str());
  assert(itr != tasks_.end());
  return itr->second;
}

void De10Rewrite::TaskMangle::visit(const EofExpression* ee) {
  // This is a bit confusing: the de10 compiler has created an entry in the
  // variable table for the argument to this expression (like we do with
  // arguments to display statements). Prior to transfering control to the fpga
  // we'll place the result of this eof check into this location in hardware.
  const auto itr = de_->table_find(ee->get_arg());
  assert(itr != de_->table_end());

  stringstream ss;
  TextPrinter(ss) << ee;
  tasks_[ss.str()] = new Identifier(new Id("__var"), new Number(Bits(32, itr->second.index())));
}

void De10Rewrite::TaskMangle::visit(const Identifier* id) {
  if (!within_task_) {
    return;
  }

  const auto titr = de_->table_find(id);
  assert(titr != de_->table_end());

  // This is a bit nasty. The amount of space we set aside for this argument in
  // the variable table may exceed its actual bit-width. This is because the
  // width of the argument may have been implicitly extended if it's part of an
  // expression. 
  const auto* r = Resolve().get_resolution(id);
  assert(r != nullptr);
  const auto w = Evaluate().get_width(r);

  for (size_t i = 0; i < titr->second.entry_size(); ++i) {
    const auto upper = min(32*(i+1),w)-1;
    const auto lower = 32*i;

    // Create a sign extension mask: all zeros for unsigned values, 32 copies
    // of id's highest order bit for signed values.
    Expression* sext = nullptr;
    if (Evaluate().get_signed(id)) {
      sext = new MultipleConcatenation(
        new Number("32"),
        new Concatenation((w == 1) ?
          new Identifier(id->front_ids()->clone()) :
          new Identifier(id->front_ids()->clone(), new Number(Bits(32, w-1)))
        )
      );
    } else {
      sext = new Number(Bits(32, 0), Number::Format::HEX);
    }

    // Concatenate the rhs with the sign extension bits
    auto* lsbs = new Identifier(id->front_ids()->clone());
    id->clone_dim(lsbs->back_inserter_dim());
    if (lsbs->size_dim() > r->size_dim()) {
      lsbs->purge_to_dim(r->size_dim());
    }
    if (upper == lower) {
      lsbs->push_back_dim(new Number(Bits(32, upper)));
    } else if (upper > lower) {
      lsbs->push_back_dim(new RangeExpression(upper+1, lower));
    } 
    auto* rhs = new Concatenation(sext);
    rhs->push_back_exprs(lsbs);

    // Attach the concatenation to an assignment, we'll always have enough bits now
    t_->push_back_stmts(new NonblockingAssign(
      new VariableAssign(
        new Identifier(new Id("__var"), new Number(Bits(32, titr->second.index()+i))),
        rhs
      )
    ));
  }
}

void De10Rewrite::TaskMangle::visit(const DisplayStatement* ds) {
  begin_mangle_task();
  ds->accept_args(this);
  finish(ds);
}

void De10Rewrite::TaskMangle::visit(const ErrorStatement* es) {
  begin_mangle_task();
  es->accept_args(this);
  finish(es);
}

void De10Rewrite::TaskMangle::visit(const FinishStatement* fs) {
  begin_mangle_task();
  fs->accept_arg(this);
  finish(fs);
}

void De10Rewrite::TaskMangle::visit(const GetStatement* gs) {
  (void) gs;
  begin_mangle_io();
  finish(gs);
}

void De10Rewrite::TaskMangle::visit(const InfoStatement* is) {
  begin_mangle_task();
  is->accept_args(this);
  finish(is);
}

void De10Rewrite::TaskMangle::visit(const PutStatement* ps) {
  (void) ps;
  begin_mangle_io();
  finish(ps);
}

void De10Rewrite::TaskMangle::visit(const RestartStatement* rs) {
  (void) rs;
  begin_mangle_task();
  finish(rs);
}

void De10Rewrite::TaskMangle::visit(const RetargetStatement* rs) {
  (void) rs;
  begin_mangle_task();
  finish(rs);
}

void De10Rewrite::TaskMangle::visit(const SaveStatement* ss) {
  (void) ss;
  begin_mangle_task();
  finish(ss);
}

void De10Rewrite::TaskMangle::visit(const SeekStatement* ss) {
  (void) ss;
  begin_mangle_io();
  finish(ss);
}

void De10Rewrite::TaskMangle::visit(const WarningStatement* ws) {
  begin_mangle_task();
  ws->accept_args(this);
  finish(ws);
}

void De10Rewrite::TaskMangle::visit(const WriteStatement* ws) {
  begin_mangle_task();
  ws->accept_args(this);
  finish(ws);
}

void De10Rewrite::TaskMangle::begin_mangle_io() {
  within_task_ = true;
  t_ = new SeqBlock();
  t_->push_back_stmts(new NonblockingAssign(
    new VariableAssign(
      new Identifier(
        new Id("__next_io_mask"),
        new Number(Bits(32, io_idx_))
      ),
      new UnaryExpression(
        UnaryExpression::Op::TILDE,
        new Identifier(
          new Id("__next_io_mask"),
          new Number(Bits(32, io_idx_))
        )
      )
    )    
  ));
  ++io_idx_;
}

void De10Rewrite::TaskMangle::begin_mangle_task() {
  within_task_ = true;
  t_ = new SeqBlock();
  t_->push_back_stmts(new NonblockingAssign(
    new VariableAssign(
      new Identifier(
        new Id("__next_task_mask"),
        new Number(Bits(32, task_idx_))
      ),
      new UnaryExpression(
        UnaryExpression::Op::TILDE,
        new Identifier(
          new Id("__next_task_mask"),
          new Number(Bits(32, task_idx_))
        )
      )
    )    
  ));
  ++task_idx_;
}

void De10Rewrite::TaskMangle::finish(const SystemTaskEnableStatement* s) {
  within_task_ = false;
  stringstream ss;
  TextPrinter(ss) << s;
  tasks_[ss.str()] = new ConditionalStatement(new Identifier("__live"), t_, new SeqBlock());
}

De10Rewrite::RewriteText::RewriteText(const ModuleDeclaration* md, const De10Logic* de) : Builder() { 
  md_ = md;
  de_ = de;
}

Attributes* De10Rewrite::RewriteText::build(const Attributes* as) {
  (void) as;
  return nullptr;
}

ModuleItem* De10Rewrite::RewriteText::build(const RegDeclaration* rd) {
  return ModuleInfo(md_).is_stateful(rd->get_id()) ? nullptr : rd->clone();
}

ModuleItem* De10Rewrite::RewriteText::build(const PortDeclaration* pd) {
  ModuleInfo info(md_);
  if (info.is_stateful(pd->get_decl()->get_id()) || info.is_input(pd->get_decl()->get_id())) {
    return nullptr;
  } else {
    return pd->get_decl()->clone();
  }
}

Statement* De10Rewrite::RewriteText::build(const NonblockingAssign* na) {
  // Create empty blocks for true and false branches (we'll never populate the
  // false branch)
  auto* t = new SeqBlock();
  auto* f = new SeqBlock();

  // Look up the target of this assignment and the indices it spans in the
  // variable table
  const auto* lhs = na->get_assign()->get_lhs();
  const auto* r = Resolve().get_resolution(lhs);
  assert(r != nullptr);
  
  // Replace the original assignment with an assignment to a temporary variable
  auto* next = lhs->clone();
  next->purge_ids();
  next->push_back_ids(new Id(lhs->front_ids()->get_readable_sid() + "_next"));
  t->push_back_stmts(new NonblockingAssign(
    na->clone_ctrl(),
    new VariableAssign(
      next,
      na->get_assign()->get_rhs()->clone()
    )
  ));

  // Insert a new assignment to the next mask
  t->push_back_stmts(new NonblockingAssign(
    new VariableAssign(
      new Identifier(
        new Id("__next_update_mask"),
        get_table_range(r, lhs)
      ),
      new UnaryExpression(
        UnaryExpression::Op::TILDE,
        new Identifier(
          new Id("__next_update_mask"),
          get_table_range(r, lhs)
        )
      )
    )
  ));

  // Return a conditional statement in place of the original assignment
  return new ConditionalStatement(new Identifier("__live"), t, f);
}

Expression* De10Rewrite::RewriteText::get_table_range(const Identifier* r, const Identifier* i) {
  // Look up r in the variable table
  const auto titr = de_->table_find(r);
  assert(titr != de_->table_end());
  assert(titr->second.materialized());

  // Start with an expression for where this variable begins in the variable table
  Expression* idx = new Number(Bits(32, titr->second.index()));

  // Now iterate over the arity of r and compute a symbolic expression 
  auto mul = titr->second.elements();
  auto iitr = i->begin_dim();
  for (auto a : titr->second.arity()) {
    mul /= a;
    idx = new BinaryExpression(
      idx,
      BinaryExpression::Op::PLUS,
      new BinaryExpression(
        (*iitr++)->clone(),
        BinaryExpression::Op::TIMES,
        new Number(Bits(32, mul*titr->second.element_size()))
      )
    );
  }
  return new RangeExpression(idx, RangeExpression::Type::PLUS, new Number(Bits(32, titr->second.element_size())));
}

De10Rewrite::Machinify::Machinify() : Editor() { 
  idx_ = 0;
}

void De10Rewrite::Machinify::edit(AlwaysConstruct* ac) {
  assert(ac->get_stmt()->is(Node::Tag::timing_control_statement));
  const auto* tcs = static_cast<const TimingControlStatement*>(ac->get_stmt());
  assert(tcs->get_ctrl()->is(Node::Tag::event_control));
  const auto* ec = static_cast<const EventControl*>(tcs->get_ctrl());

  auto* ctrl = ec->clone();
  ctrl->push_back_events(new Event(Event::Type::POSEDGE, new Identifier("__resume")));
  auto* machine = Generate(idx_++).run(tcs->get_stmt());

  ac->replace_stmt(new TimingControlStatement(ctrl, machine));
}

De10Rewrite::Machinify::Generate::Generate(size_t idx) : Visitor() { 
  idx_ = idx;
}

CaseStatement* De10Rewrite::Machinify::Generate::run(const Statement* s) {
  machine_ = new CaseStatement(CaseStatement::Type::CASE, state_var());
  next_state();
  s->accept(this);
  return machine_;
}

void De10Rewrite::Machinify::Generate::visit(const BlockingAssign* ba) {
  append(ba);
}

void De10Rewrite::Machinify::Generate::visit(const NonblockingAssign* na) {
  append(na);
}

void De10Rewrite::Machinify::Generate::visit(const SeqBlock* sb) {
  sb->accept_stmts(this);
}

void De10Rewrite::Machinify::Generate::visit(const CaseStatement* cs) {
  const auto begin = current();

  vector<pair<size_t, SeqBlock*>> begins;
  vector<pair<size_t, SeqBlock*>> ends;
  for (auto i = cs->begin_items(), ie = cs->end_items(); i != ie; ++i) {
    next_state();
    begins.push_back(current());
    (*i)->accept_stmt(this);
    ends.push_back(current());
  }

  auto* branch = new CaseStatement(cs->get_type(), cs->get_cond()->clone());
  size_t idx = 0;
  for (auto i = cs->begin_items(), ie = cs->end_items(); i != ie; ++i) {
    branch->push_back_items(new CaseItem(
      new NonblockingAssign(new VariableAssign(
        new Identifier("state"),
        new Number(Bits(32, begins[idx++].first))
      ))
    ));
    for (auto j = (*i)->begin_exprs(), je = (*i)->end_exprs(); j != je; ++j) {
      branch->back_items()->push_back_exprs((*j)->clone());
    }
  }
  append(begin.second, branch);
  
  next_state();
  for (auto& e : ends) {
    transition(e.second, current().first);
  }
}

void De10Rewrite::Machinify::Generate::visit(const ConditionalStatement* cs) {
  const auto begin = current();

  next_state();
  const auto then_begin = current();
  cs->get_then()->accept(this);
  const auto then_end = current();
  
  next_state();
  const auto else_begin = current();
  cs->get_else()->accept(this);
  const auto else_end = current();
  
  auto* branch = new ConditionalStatement(
    cs->get_if()->clone(),
    new NonblockingAssign(new VariableAssign(
      new Identifier("state"),
      new Number(Bits(32, then_begin.first))
    )),
    new NonblockingAssign(new VariableAssign(
      new Identifier("state"),
      new Number(Bits(32, else_begin.first))
    ))
  );
  append(begin.second, branch);

  next_state();
  transition(then_end.second, current().first);
  transition(else_end.second, current().first);
}

void De10Rewrite::Machinify::Generate::visit(const DisplayStatement* ds) {
  append(ds);
}

void De10Rewrite::Machinify::Generate::visit(const ErrorStatement* es) {
  append(es);
}

void De10Rewrite::Machinify::Generate::visit(const FinishStatement* fs) {
  append(fs);
}

void De10Rewrite::Machinify::Generate::visit(const GetStatement* gs) {
  append(gs);
  transition(current().first+1);
  next_state();
}

void De10Rewrite::Machinify::Generate::visit(const InfoStatement* is) {
  append(is);
}

void De10Rewrite::Machinify::Generate::visit(const PutStatement* ps) {
  append(ps);
}

void De10Rewrite::Machinify::Generate::visit(const RestartStatement* rs) {
  append(rs);
}

void De10Rewrite::Machinify::Generate::visit(const RetargetStatement* rs) {
  append(rs);
}

void De10Rewrite::Machinify::Generate::visit(const SaveStatement* ss) {
  append(ss);
}

void De10Rewrite::Machinify::Generate::visit(const SeekStatement* ss) {
  append(ss);
  transition(current().first+1);
  next_state();
}

void De10Rewrite::Machinify::Generate::visit(const WarningStatement* ws) {
  append(ws);
}

void De10Rewrite::Machinify::Generate::visit(const WriteStatement* ws) {
  append(ws);
}

pair<size_t, SeqBlock*> De10Rewrite::Machinify::Generate::current() const {
  const auto n = machine_->size_items()-1;
  auto* sb = static_cast<SeqBlock*>(machine_->back_items()->get_stmt());
  return make_pair(n, sb);
}

Identifier* De10Rewrite::Machinify::Generate::state_var() const {
  stringstream ss;
  ss << "__state_" << idx_;
  return new Identifier(ss.str());
}

void De10Rewrite::Machinify::Generate::append(const Statement* s) {
  auto* sb = static_cast<SeqBlock*>(machine_->back_items()->get_stmt());
  append(sb, s);
}

void De10Rewrite::Machinify::Generate::append(SeqBlock* sb, const Statement* s) {
  sb->push_back_stmts(s->clone());
}

void De10Rewrite::Machinify::Generate::transition(size_t n) {
  auto* sb = static_cast<SeqBlock*>(machine_->back_items()->get_stmt());
  transition(sb, n);
}

void De10Rewrite::Machinify::Generate::transition(SeqBlock* sb, size_t n) {
  sb->push_back_stmts(new NonblockingAssign(new VariableAssign(
    state_var(),
    new Number(Bits(32, n))
  )));
}

void De10Rewrite::Machinify::Generate::next_state() {
  auto* ci = new CaseItem(new SeqBlock());
  ci->push_back_exprs(new Number(Bits(32, machine_->size_items())));
  machine_->push_back_items(ci);
}

De10Rewrite::FinishMangle::FinishMangle(TaskMangle* tm) : Rewriter() {
  tm_ = tm;
}

Expression* De10Rewrite::FinishMangle::rewrite(EofExpression* ee) {
  return static_cast<Expression*>(tm_->get(ee));
}

Statement* De10Rewrite::FinishMangle::rewrite(DisplayStatement* ds) {
  return static_cast<Statement*>(tm_->get(ds));
}

Statement* De10Rewrite::FinishMangle::rewrite(ErrorStatement* es) {
  return static_cast<Statement*>(tm_->get(es));
}

Statement* De10Rewrite::FinishMangle::rewrite(FinishStatement* fs) {
  return static_cast<Statement*>(tm_->get(fs));
}

Statement* De10Rewrite::FinishMangle::rewrite(GetStatement* gs) {
  return static_cast<Statement*>(tm_->get(gs));
}

Statement* De10Rewrite::FinishMangle::rewrite(InfoStatement* is) {
  return static_cast<Statement*>(tm_->get(is));
}

Statement* De10Rewrite::FinishMangle::rewrite(PutStatement* ps) {
  return static_cast<Statement*>(tm_->get(ps));
}

Statement* De10Rewrite::FinishMangle::rewrite(RestartStatement* rs) {
  return static_cast<Statement*>(tm_->get(rs));
}

Statement* De10Rewrite::FinishMangle::rewrite(RetargetStatement* rs) {
  return static_cast<Statement*>(tm_->get(rs));
}

Statement* De10Rewrite::FinishMangle::rewrite(SaveStatement* ss) {
  return static_cast<Statement*>(tm_->get(ss));
}

Statement* De10Rewrite::FinishMangle::rewrite(SeekStatement* ss) {
  return static_cast<Statement*>(tm_->get(ss));
}

Statement* De10Rewrite::FinishMangle::rewrite(WarningStatement* ws) {
  return static_cast<Statement*>(tm_->get(ws));
}

Statement* De10Rewrite::FinishMangle::rewrite(WriteStatement* ws) {
  return static_cast<Statement*>(tm_->get(ws));
}

} // namespace cascade
