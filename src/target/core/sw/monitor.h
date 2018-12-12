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

#ifndef CASCADE_SRC_TARGET_CORE_SW_MONITOR_H
#define CASCADE_SRC_TARGET_CORE_SW_MONITOR_H

#include <cassert>
#include "src/verilog/analyze/read_set.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/ast/visitors/editor.h"

namespace cascade {

class Monitor : public Editor {
  public:
    Monitor();
    ~Monitor() override = default;

    void init(ModuleItem* mi);

  private:
    void wait_on_node(Node* n, Node* m);
    void wait_on_reads(Node* n, Expression* e);

    void edit(Event* e) override;
    void edit(AlwaysConstruct* ac) override;
    void edit(ContinuousAssign* ca) override;
    void edit(ParBlock* pb) override;
    void edit(SeqBlock* sb) override;
    void edit(CaseStatement* cs) override;
    void edit(ConditionalStatement* cs) override;
    void edit(ForStatement* fs) override;
    void edit(RepeatStatement* rs) override;
    void edit(WhileStatement* ws) override;
    void edit(TimingControlStatement* tcs) override;
    void edit(WaitStatement* ws) override;
    void edit(EventControl* ec) override;
};

inline Monitor::Monitor() : Editor() { }

inline void Monitor::init(ModuleItem* mi) {
  mi->accept(this);
}

inline void Monitor::wait_on_node(Node* n, Node* m) {
  m->monitor_.push_back(n);
}

inline void Monitor::wait_on_reads(Node* n, Expression* m) {
  for (auto i : ReadSet(m)) {
    auto r = Resolve().get_resolution(i);
    assert(r != nullptr);
    wait_on_node(n, const_cast<Identifier*>(r));
  }
}

inline void Monitor::edit(Event* e) {
  // TODO: Support for complex expressions here
  auto id = dynamic_cast<Identifier*>(e->get_expr());
  assert(id != nullptr);

  auto r = Resolve().get_resolution(id);
  wait_on_node(e, const_cast<Identifier*>(r));
}

inline void Monitor::edit(AlwaysConstruct* ac) {
  Editor::edit(ac);
  wait_on_node(ac, ac->get_stmt());
}

inline void Monitor::edit(ContinuousAssign* ca) {
  Editor::edit(ca);
  wait_on_reads(ca, ca->get_assign()->get_rhs());
}

inline void Monitor::edit(ParBlock* pb) {
  Editor::edit(pb);
  for (auto s : *pb->get_stmts()) {
    wait_on_node(pb, s);
  } 
}

inline void Monitor::edit(SeqBlock* sb) {
  Editor::edit(sb);
  for (auto s : *sb->get_stmts()) {
    wait_on_node(sb, s);
  } 
}

inline void Monitor::edit(CaseStatement* cs) {
  Editor::edit(cs);
  for (auto ci : *cs->get_items()) {
    wait_on_node(cs, ci->get_stmt());
  }
}

inline void Monitor::edit(ConditionalStatement* cs) {
  Editor::edit(cs);
  wait_on_node(cs, cs->get_then());
  wait_on_node(cs, cs->get_else());
}

inline void Monitor::edit(ForStatement* fs) {
  Editor::edit(fs);
  wait_on_node(fs, fs->get_stmt());
}

inline void Monitor::edit(RepeatStatement* rs) {
  Editor::edit(rs);
  wait_on_node(rs, rs->get_stmt());
}

inline void Monitor::edit(WhileStatement* ws) {
  Editor::edit(ws);
  wait_on_node(ws, ws->get_stmt());
}

inline void Monitor::edit(TimingControlStatement* tcs) {
  Editor::edit(tcs);
  wait_on_node(tcs, tcs->get_ctrl());
  wait_on_node(tcs, tcs->get_stmt());
}

inline void Monitor::edit(WaitStatement* ws) {
  Editor::edit(ws);
  wait_on_node(ws, ws->get_stmt());
}

inline void Monitor::edit(EventControl* ec) {
  if (ec->get_events()->empty()) {
    auto tcs = dynamic_cast<TimingControlStatement*>(ec->get_parent());
    assert(tcs != nullptr);
    for (auto i : ReadSet(tcs->get_stmt())) {
      auto r = Resolve().get_resolution(i);
      assert(r != nullptr);
      wait_on_node(ec, const_cast<Identifier*>(r));
    }
    return;
  }

  Editor::edit(ec);
  for (auto e : *ec->get_events()) {
    wait_on_node(ec, e);
  }
}

} // namespace cascade

#endif
