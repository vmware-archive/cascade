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

#include <cassert>
#include <tuple>
#include <utility>
#include <vector>
#include "src/base/bits/bits.h"
#include "src/verilog/analyze/resolve.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/ast/visitors/editor.h"

namespace cascade {

// This class implements the semantics of expression evaluation as described in
// the 2005 Verilog spec. This class requires up-to-date resolution decorations
// (see resolve.h) to function correctly.

// Internally, this class relies on the fact that AST expressions are decorated
// with Bits arrays. In practice, most expressions will only have a single
// element in that array. The only exception are identifiers which are nested
// within declarations. Verilog does not allow array slicing, so wherever else
// an identifier appears, it either refers to a scalar, or a scalar element of
// an array.

// While verilog allows multidimensional arrays, expressions are decorated with
// single dimensional arrays. The mapping is standard: elements are stored in
// lexicographically ascending order. The array r[1:0][1:0] would be linearized
// as follows: r[0][0] r[0][1] r[1][0] r[1][1].

class Evaluate : public Editor {
  public:
    ~Evaluate() override = default;

    // Returns true for scalars.
    bool is_scalar(const Identifier* id);
    // Returns true for arrays.
    bool is_array(const Identifier* id);
    // Returns the arity of an expression: an empty vector for scalars, one 
    // value for the length of each dimension for arrays.
    std::vector<size_t> get_arity(const Identifier* id);

    // Returns the bit-width of the values in an expression. Returns the same
    // value for scalars and arrays.
    size_t get_width(const Expression* e);
    // Returns true if the values in an expression are signed or unsigned.
    // Returns the same for scalars and arrays.
    bool get_signed(const Expression* e);

    // Returns the bit value of an expression. Invoking this method on an expression
    // which evaluates to an array returns the first element of that array.
    const Bits& get_value(const Expression* e);
    // Returns the array value of an identifier. Invoking this method on an identifier
    // which evaluates to a scalar returns a single element array.
    const std::vector<Bits>& get_array_value(const Identifier* i);
    // Returns upper and lower values for ranges, get_value() twice otherwise.
    std::pair<size_t, size_t> get_range(const Expression* e);

    // High-level interface: Resolves id and sets the value of its target val.
    // Invoking this method on an unresolvable id or one which refers to an
    // array is undefined.
    void assign_value(const Identifier* id, const Bits& val);
    // High-level interface: Resolves id and sets the value of its target to
    // val. Invoking this method on an unresolvable id or one which refers to
    // an array subscript is undefined.
    void assign_array_value(const Identifier* id, const std::vector<Bits>& val);

    // Low-level interface: Returns an index into r's underlying array, as well
    // as bit range, based on the expressions in i's dimensions. This method is
    // undefined if i does not resolve to r. Returns -1 -1 for range to
    // indicate that no slice was provided.
    std::tuple<size_t,int,int> dereference(const Identifier* r, const Identifier* i);
    // Low-level interface: Sets the value of the ss'th element in id's
    // underlying array. Note that this value is set *in place*. This method
    // DOES NOT resolve id and then update the value which it finds there.
    void assign_value(const Identifier* id, size_t idx, int msb, int lsb, const Bits& val);
    // Low-level interface: Sets the value of the idx'th element in id's
    // underlying array. Note that this value is set *in place*. This method
    // DOES NOT resolve id and then update the value which it finds there.
    template <typename B>
    void assign_word(const Identifier* id, size_t idx, size_t n, B b);

    // Invalidates bits, size, and type for this expression and the
    // sub-expressions that it consists of.
    void invalidate(const Expression* e);

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
    void edit(UnaryExpression* ue) override;

    // Helper Methods:
    //
    // Returns the root of the expression tree containing e. See implementation
    // notes for what counts as a boundary between trees.
    const Node* get_root(const Expression* e) const;
    // Initializes the bit value associated with an identifier using the rules
    // of self- and context- determination to determine bit-width and sign.
    void init(Expression* e);
    // Updates the needs_update_ flag for this identifier and its dependencies.
    void flag_changed(const Identifier* id);

    // Invalidates bit, size, and type info for the expressions in this subtree
    struct Invalidate : public Editor {
      ~Invalidate() override = default;
      void edit(BinaryExpression* be) override;
      void edit(ConditionalExpression* ce) override;
      void edit(NestedExpression* ne) override;
      void edit(Concatenation* c) override;
      void edit(Identifier* id) override;
      void edit(MultipleConcatenation* mc) override;
      void edit(Number* n) override;
      void edit(String* s) override;
      void edit(UnaryExpression* ue) override;
      void edit(GenvarDeclaration* gd) override;
      void edit(IntegerDeclaration* id) override;
      void edit(LocalparamDeclaration* ld) override;
      void edit(NetDeclaration* nd) override; 
      void edit(ParameterDeclaration* pd) override;
      void edit(RegDeclaration* rd) override;
      void edit(VariableAssign* va) override;
    };
    // Uses the self-determination to allocate bits, sizes, and types.
    struct SelfDetermine : public Editor {
      ~SelfDetermine() override = default;
      void edit(BinaryExpression* be) override;
      void edit(ConditionalExpression* ce) override;
      void edit(NestedExpression* ne) override;
      void edit(Concatenation* c) override;
      void edit(Identifier* id) override;
      void edit(MultipleConcatenation* mc) override;
      void edit(Number* n) override;
      void edit(String* s) override;
      void edit(UnaryExpression* ue) override;
      void edit(GenvarDeclaration* gd) override;
      void edit(IntegerDeclaration* id) override;
      void edit(LocalparamDeclaration* ld) override;
      void edit(NetDeclaration* nd) override; 
      void edit(ParameterDeclaration* pd) override;
      void edit(RegDeclaration* rd) override;
      void edit(VariableAssign* va) override;
    };
    // Propagates bit-width for context determined operators.
    struct ContextDetermine : public Editor {
      ~ContextDetermine() override = default;
      void edit(BinaryExpression* be) override;
      void edit(ConditionalExpression* ce) override;
      void edit(NestedExpression* ne) override;
      void edit(Concatenation* c) override;
      void edit(Identifier* id) override;
      void edit(MultipleConcatenation* mc) override;
      void edit(Number* n) override;
      void edit(String* s) override;
      void edit(UnaryExpression* ue) override;
      void edit(GenvarDeclaration* gd) override;
      void edit(IntegerDeclaration* id) override;
      void edit(LocalparamDeclaration* ld) override;
      void edit(NetDeclaration* nd) override; 
      void edit(ParameterDeclaration* pd) override;
      void edit(RegDeclaration* rd) override;
      void edit(VariableAssign* va) override;
    };
};

template <typename B>
inline void Evaluate::assign_word(const Identifier* id, size_t idx, size_t n, B b) {
  init(const_cast<Identifier*>(id));
  assert(idx < id->bit_val_.size());
  const_cast<Identifier*>(id)->bit_val_[idx].write_word<B>(n, b);
  flag_changed(id);
}

} // namespace cascade

#endif
