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

#ifndef CASCADE_SRC_VERILOG_ANALYZE_READ_SET_H
#define CASCADE_SRC_VERILOG_ANALYZE_READ_SET_H

#include <unordered_set>
#include "verilog/ast/ast_fwd.h"
#include "verilog/ast/visitors/visitor.h"

namespace cascade {

// This class is used to record the set of variables and system task
// expressions which can determine the value of an expression.  For example,
// the expression x + y - $eof(z) depends on the values of x, y, and z, and
// also the stream pointed to by the value of z. This class is not currently on
// any critical paths, so it doesn't attempt to cache its results in the AST.

class ReadSet : public Visitor {
  public:
    // Typedefs:
    typedef std::unordered_set<const Expression*>::const_iterator const_iterator;

    // Constructors:
    explicit ReadSet(const Expression* e);
    explicit ReadSet(const Statement* s);
    ~ReadSet() override = default;

    // Iterator Interface:
    const_iterator begin() const;
    const_iterator end() const;

  private:
    std::unordered_set<const Expression*> reads_;
    bool lhs_;

    // Editor Interface:
    void visit(const FeofExpression* fe) override;
    void visit(const Identifier* id) override;
    void visit(const VariableAssign* va) override;
};

} // namespace cascade

#endif
