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

#include "verilog/transform/event_expand.h"

#include <map>
#include <sstream>
#include <string>
#include "verilog/analyze/read_set.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"
#include "verilog/print/print.h"

using namespace std;

namespace cascade {

EventExpand::EventExpand() : Editor() { }

void EventExpand::run(ModuleDeclaration* md) {
  md->accept(this);
}

void EventExpand::edit(TimingControlStatement* tcs) {
  // Proceed as normal for everything other than an empty event control
  if (!tcs->get_ctrl()->is(Node::Tag::event_control)) {
    return Editor::edit(tcs);
  }
  const auto* ec = static_cast<const EventControl*>(tcs->get_ctrl());
  if (!ec->empty_events()) {
    return Editor::edit(tcs);
  }

  // Look up the variable dependencies for this statement. We don't *currently*
  // support system task statements here. Sort lexicographically to ensure
  // deterministic compiles.
  map<string, const Identifier*> reads;
  for (auto* i : ReadSet(tcs->get_stmt())) {
    if (i->is(Node::Tag::identifier)) {
      const auto* r = Resolve().get_resolution(static_cast<const Identifier*>(i));
      assert(r != nullptr);

      stringstream ss;
      ss << r;
      reads.insert(make_pair(ss.str(), r));
    }
  }

  // Create a new timing control with an explicit list
  auto* ctrl = new EventControl();
  for (auto& r : reads) {
    auto* e = r.second->clone();
    e->purge_dim();
    ctrl->push_back_events(new Event(Event::Type::EDGE, e));
  }
  tcs->replace_ctrl(ctrl);
}

} // namespace cascade
