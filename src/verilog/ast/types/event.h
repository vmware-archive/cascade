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

#ifndef CASCADE_SRC_VERILOG_AST_EVENT_H
#define CASCADE_SRC_VERILOG_AST_EVENT_H

#include "src/verilog/ast/types/expression.h"
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/node.h"

namespace cascade {

class Event : public Node {
  public:
    // Supporting Concepts:
    enum class Type : uint8_t {
      EDGE = 0,
      NEGEDGE,
      POSEDGE
    };

    // Constructors:
    Event(Type type__, Expression* expr__);
    ~Event() override;

    // Node Interface:
    NODE(Event)
    Event* clone() const override;

    // Get/Set:
    VAL_GET_SET(Event, Type, type)
    PTR_GET_SET(Event, Expression, expr)

  private:
    VAL_ATTR(Type, type);
    PTR_ATTR(Expression, expr);
};

inline Event::Event(Type type__, Expression* expr__) : Node(Node::Tag::event) {
  VAL_SETUP(type);
  PTR_SETUP(expr);
  parent_ = nullptr;
}

inline Event::~Event() {
  VAL_TEARDOWN(type);
  PTR_TEARDOWN(expr);
}

inline Event* Event::clone() const {
  return new Event(type_, expr_->clone());
}

} // namespace cascade 

#endif
