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

#include "verilog/transform/control_merge.h"

#include <unordered_map>
#include <sstream>
#include <string>
#include <vector>
#include "verilog/analyze/module_info.h"
#include "verilog/analyze/navigate.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"
#include "verilog/print/text/text_printer.h"

using namespace std;

namespace cascade {

void ControlMerge::run(ModuleDeclaration* md) {
  // Index initial and always blocks
  vector<const Statement*> initial_index;
  unordered_map<string, vector<const TimingControlStatement*>> always_index;
  for (auto i = md->begin_items(); i != md->end_items(); ++i) {
    switch ((*i)->get_tag()) {
      case Node::Tag::initial_construct: {
        const auto* ic = static_cast<const InitialConstruct*>(*i);
        initial_index.push_back(ic->get_stmt());
        break;
      }
      case Node::Tag::always_construct: {
        const auto* ac = static_cast<const AlwaysConstruct*>(*i);
        assert(ac->get_stmt()->is_subclass_of(Node::Tag::timing_control_statement));
        const auto* tcs = static_cast<const TimingControlStatement*>(ac->get_stmt());
        stringstream ss;
        TextPrinter(ss) << tcs->get_ctrl();
        always_index[ss.str()].push_back(tcs);
        break;
      }
      default:
        break;
    }
  }

  // Create canonical initial block
  auto* sb = new SeqBlock();
  for (auto* s : initial_index) {
    sb->push_back_stmts(s->clone());
  }
  auto* initial = new InitialConstruct(new Attributes(), sb); 

  // Create canonical always blocks
  vector<AlwaysConstruct*> always;
  for (const auto& a : always_index) {
    auto* sb = new SeqBlock();
    for (auto* tcs : a.second) {
      sb->push_back_stmts(tcs->get_stmt()->clone());
    }
    auto* tcs = new TimingControlStatement(a.second[0]->get_ctrl()->clone(), sb);
    always.push_back(new AlwaysConstruct(tcs));
  }

  // Delete original control statements
  for (auto i = md->begin_items(); i != md->end_items();) {
    switch ((*i)->get_tag()) {
      case Node::Tag::initial_construct:
      case Node::Tag::always_construct:
        i = md->purge_items(i);
        break;
      default:
        ++i;
        break;
    }
  }

  // Append canonical instances 
  md->push_back_items(initial);
  for (auto* a : always) {
    md->push_back_items(a); 
  }

  Resolve().invalidate(md);
  Navigate(md).invalidate();
  ModuleInfo(md).invalidate();
}

} // namespace cascade
