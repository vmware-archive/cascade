// Copyright 2017-2018 VMware, Inc.
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

#ifndef CASCADE_SRC_FPGA_VERILOG_ANALYZE_BIT_WIDTH_H
#define CASCADE_SRC_FPGA_VERILOG_ANALYZE_BIT_WIDTH_H

#include <stddef.h>
#include "src/verilog/ast/ast_fwd.h"
#include "src/verilog/ast/visitors/visitor.h"

namespace cascade {

// This class implements the semantics of Table 5-22 from the Verilog 2k5 Spec.
// It does not store any decorations in the AST and has dependence on other
// decorations. It can be thought of as a pure function object.

class BitWidth : public Visitor {
  public:
    // Constructors:
    ~BitWidth() override = default;

    // Returns the bit-width of the value of an expression.
    size_t get_width(const Expression* e);

  private:
    size_t res_;

    // Visitor Interface:
    void visit(const BinaryExpression* be) override;
    void visit(const ConditionalExpression* ce) override;
    void visit(const Concatenation* c) override;
    void visit(const Identifier* id) override;
    void visit(const MultipleConcatenation* mc) override;
    void visit(const Number* n) override;
    void visit(const String* s) override;
    void visit(const UnaryExpression* ue) override;

    // Helper Methods:
    size_t get_decl_width(const Identifier* id);
    size_t get_decl_width(const RangeExpression* id);
};

} // namespace cascade

#endif
