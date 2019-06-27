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

#ifndef CASCADE_SRC_VERILOG_ANALYZE_EVALUATE_H
#define CASCADE_SRC_VERILOG_ANALYZE_EVALUATE_H

#include <cassert>
#include <functional>
#include <tuple>
#include <utility>
#include <vector>
#include "common/bits.h"
#include "common/vector.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"
#include "verilog/ast/visitors/editor.h"

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
    // Typedefs:
    typedef std::function<bool(Evaluate*, const FeofExpression*)> FeofHandler;
    typedef std::function<uint32_t(Evaluate*, const FopenExpression*)> FopenHandler;

    // Constructors:
    Evaluate();
    ~Evaluate() override = default;

    // Configuration Interface:
    Evaluate& set_feof_handler(FeofHandler h);
    Evaluate& set_fopen_handler(FopenHandler h);

    // Returns the arity of an expression: an empty vector for scalars, one 
    // value for the length of each dimension for arrays.
    std::vector<size_t> get_arity(const Identifier* id);

    // Returns the bit-width of the value of an expression. Returns the same
    // value for scalars and arrays.
    size_t get_width(const Expression* e);
    // Returns the underlying type of an expression. Returns the same for
    // scalars and arrays.
    Bits::Type get_type(const Expression* e);

    // Returns the bit value of an expression. Invoking this method on an expression
    // which evaluates to an array returns the first element of that array.
    const Bits& get_value(const Expression* e);
    // Returns the array value of an identifier. Invoking this method on an identifier
    // which evaluates to a scalar returns a single element array.
    const Vector<Bits>& get_array_value(const Identifier* i);
    // Returns upper and lower values for ranges, get_value() twice otherwise.
    std::pair<size_t, size_t> get_range(const Expression* e);

    // High-level interface: Resolves id and sets the value of its target val.
    // Invoking this method on an unresolvable id or one which refers to an
    // array is undefined.
    void assign_value(const Identifier* id, const Bits& val);
    // High-level interface: Resolves id and sets the value of its target to
    // val. Invoking this method on an unresolvable id or one which refers to
    // an array subscript is undefined.
    void assign_array_value(const Identifier* id, const Vector<Bits>& val);

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

    // Forced a recomputation for the next evaluation of any expression that
    // depends on this variable.
    void flag_changed(const Identifier* id);
    // Forces a recomputation for the next evaluation of this expression or any
    // expression that depends on this one.
    void flag_changed(const FeofExpression* fe);
    // Invalidates bits, size, and type for this expression and the
    // sub-expressions that it consists of.
    void invalidate(const Expression* e);

  private:
    // Target-specific handlers:
    FeofHandler feof_;
    FopenHandler fopen_;

    // Editor Interface:
    void edit(BinaryExpression* be) override;
    void edit(ConditionalExpression* ce) override;
    void edit(FeofExpression* fe) override;
    void edit(FopenExpression* fe) override;
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

    // Invalidates bit, size, and type info for the expressions in this subtree
    struct Invalidate : Editor {
      ~Invalidate() override = default;
      void edit(BinaryExpression* be) override;
      void edit(ConditionalExpression* ce) override;
      void edit(FeofExpression* fe) override;
      void edit(FopenExpression* fe) override;
      void edit(Concatenation* c) override;
      void edit(Identifier* id) override;
      void edit(MultipleConcatenation* mc) override;
      void edit(Number* n) override;
      void edit(String* s) override;
      void edit(UnaryExpression* ue) override;
      void edit(LocalparamDeclaration* ld) override;
      void edit(NetDeclaration* nd) override; 
      void edit(ParameterDeclaration* pd) override;
      void edit(RegDeclaration* rd) override;
    };
    // Uses self-determination to allocate bits, sizes, and types.
    struct SelfDetermine : Editor {
      SelfDetermine(Evaluate* eval);
      ~SelfDetermine() override = default;
      void edit(BinaryExpression* be) override;
      void edit(ConditionalExpression* ce) override;
      void edit(FeofExpression* fe) override;
      void edit(FopenExpression* fe) override;
      void edit(Concatenation* c) override;
      void edit(Identifier* id) override;
      void edit(MultipleConcatenation* mc) override;
      void edit(Number* n) override;
      void edit(String* s) override;
      void edit(UnaryExpression* ue) override;
      void edit(GenvarDeclaration* gd) override;
      void edit(LocalparamDeclaration* ld) override;
      void edit(NetDeclaration* nd) override; 
      void edit(ParameterDeclaration* pd) override;
      void edit(RegDeclaration* rd) override;
      Evaluate* eval_;
    };
    // Propagates bit-width for context determined operators.
    struct ContextDetermine : Editor {
      ContextDetermine(Evaluate* eval);
      ~ContextDetermine() override = default;
      void edit(BinaryExpression* be) override;
      void edit(ConditionalExpression* ce) override;
      void edit(FeofExpression* fe) override;
      void edit(FopenExpression* fe) override;
      void edit(Identifier* id) override;
      void edit(MultipleConcatenation* mc) override;
      void edit(Number* n) override;
      void edit(String* s) override;
      void edit(UnaryExpression* ue) override;
      void edit(GenvarDeclaration* gd) override;
      void edit(ContinuousAssign* ca) override;
      void edit(LocalparamDeclaration* ld) override;
      void edit(NetDeclaration* nd) override; 
      void edit(ParameterDeclaration* pd) override;
      void edit(RegDeclaration* rd) override;
      void edit(BlockingAssign* ba) override;
      void edit(NonblockingAssign* na) override;
      void edit(VariableAssign* va) override;
      Evaluate* eval_;
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
