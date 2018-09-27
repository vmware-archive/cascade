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

#ifndef CASCADE_SRC_VERILOG_ANALYZE_EVALUATE_H
#define CASCADE_SRC_VERILOG_ANALYZE_EVALUATE_H

#include <utility>
#include "src/base/bits/bits.h"
#include "src/verilog/ast/visitors/editor.h"

namespace cascade {

// This class is used to compute the value of an expression in the AST.  It can
// be used to statically compute the value of compile-time constants (see
// constant.h).  It can also be used to store values as decorations in the AST
// to, for example, override the initial value of a register. By doing so, it
// can also be used to compute runtime-specific value computations based on the
// state of the AST. This class requires up-to-date resolution decorations (see
// resolve.h) to function correctly.

class Evaluate : public Editor {
  public:
    ~Evaluate() override = default;

    // Returns the value of an expression based on the value decorations that
    // are currently stored in the AST.
    const Bits& get_value(const Expression* e);
    // Returns the upper and lower ends of a range expression based on the
    // value decorations that are currently stored in the AST.
    std::pair<size_t, size_t> get_range(const Expression* e);

    // Assigns a value to an identifier. The behavior of this method is
    // undefined if the bit width of val exceeds the width of id.
    void assign_value(const Identifier* id, const Bits& val);
    // Assigns the idx'th bit of val to the 0th bit of id.
    void assign_value(const Identifier* id, const Bits& val, size_t idx);
    // Assigns the i-j+1 bits between i and j of val to the i-j+1th bits of id.
    // The behavior of this method is undefined if the bit width of this range
    // exceeds the range of id.
    void assign_value(const Identifier* id, const Bits& val, size_t i, size_t j);

  private:
    // Editor Interface:
    void edit(BinaryExpression* be) override;
    void edit(ConditionalExpression* ce) override;
    void edit(NestedExpression* ne) override;
    void edit(Concatenation* c) override;
    void edit(Identifier* id) override;
    void edit(MultipleConcatenation* mc) override;
    void edit(Number* n) override;
    void edit(String* s) override;
    void edit(RangeExpression* re) override;
    void edit(UnaryExpression* ue) override;
    void edit(GenvarDeclaration* gd) override;
    void edit(IntegerDeclaration* id) override;
    void edit(LocalparamDeclaration* ld) override;
    void edit(NetDeclaration* nd) override; 
    void edit(ParameterDeclaration* pd) override;
    void edit(RegDeclaration* rd) override;

    // Helper Methods:
    void set_value(const Identifier* id, const Bits& val);
    void flag_changed(const Identifier* id);
    void init_bits(const Expression* e);
};

} // namespace cascade

#endif
