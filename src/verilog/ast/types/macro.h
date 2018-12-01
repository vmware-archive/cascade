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

#ifndef CASCADE_SRC_VERILOG_AST_TYPES_MACRO_H
#define CASCADE_SRC_VERILOG_AST_TYPES_MACRO_H

// Naming Helpers
//
// Calling convention for function arguments. Variable names are suffixed with
// two trailing underscores.
#define INPUT(x) x##__
// Naming convention for private members. Attribtues are suffixed with a single
// trailing underscore.
#define PRIVATE(x) x##_

// Constructor Helpers:
//
// Binds a value attribute to an input argument.
#define VAL_SETUP(t) \
  PRIVATE(t) = INPUT(t);
// Binds a pointer attribute to an input argument. Throws an assertion
// for null pointers and sets the argument's parent pointer to this node.
#define PTR_SETUP(t) \
  assert(INPUT(t) != nullptr); \
  PRIVATE(t) = INPUT(t); \
  PRIVATE(t)->parent_ = this;
// Binds a maybe attribute to an input argument. Identical to pointer setup, but
// allows for null pointers.
#define MAYBE_SETUP(t) \
  PRIVATE(t) = INPUT(t); \
  if (PRIVATE(t) != nullptr) { \
    PRIVATE(t)->parent_ = this; \
  }
// Binds a many attribute to an input argument. Identical to value setup, as
// values are stored in non-pointer objects. Sets the parent pointer for every
// element to this node.
#define MANY_SETUP(t) \
  PRIVATE(t) = INPUT(t); \
  for (auto n : PRIVATE(t)) { \
    n->parent_ = this; \
  }

// Destructor Helpers:
//
// Frees the memory associated with a value attribute (does nothing).
#define VAL_TEARDOWN(t)
// Frees the memory associated with a pointer attribute by calling
// delete.  Throws an assertion of this attribute is a null pointer.
#define PTR_TEARDOWN(t) \
  assert(PRIVATE(t) != nullptr); \
  delete PRIVATE(t);
// Frees the memory associated with a maybe attribute. Identical to pointer
// teardown but allows for null pointers.
#define MAYBE_TEARDOWN(t) \
  if (PRIVATE(t) != nullptr) { \
    delete PRIVATE(t); \
  }
// Frees the memory associated with every element.
#define MANY_TEARDOWN(t) \
  for (auto n : PRIVATE(t)) { \
    delete n; \
  }

// Clone Implementation Helpers:
// 
// Value attributes are passed by value. 
#define VAL(x) PRIVATE(x)
// Pointer attributes are cloned before being passed as arguments.
#define PTR(x) PRIVATE(x)->clone()
// Maybe attribtues are only cloned if they are non-null.
#define MAYBE(x) (PRIVATE(x) != nullptr) ? PRIVATE(x)->clone() : nullptr
// Many attributes are cloned before being passed as arguments.
#define MANY(x) /* TODO */

// Variadic definitions for invoking clone with 1 to 6 arguments.
#define CLONE_1(T, t1) \
  T* clone() const override { \
    return new T(t1); \
  } 
#define CLONE_2(T, t1, t2) \
  T* clone() const override { \
    return new T(t1, t2); \
  }
#define CLONE_3(T, t1, t2, t3) \
  T* clone() const override { \
    return new T(t1, t2, t3); \
  }
#define CLONE_4(T, t1, t2, t3, t4) \
  T* clone() const override { \
    return new T(t1, t2, t3, t4); \
  }
#define CLONE_5(T, t1, t2, t3, t4, t5) \
  T* clone() const override { \
    return new T(t1, t2, t3, t4, t5); \
  }
#define CLONE_6(T, t1, t2, t3, t4, t5, t6) \
  T* clone() const override { \
    return new T(t1, t2, t3, t4, t5, t6); \
  }
#define GET_CLONE(_0, _1, _2, _3, _4, _5, _6, CLONE, ...) CLONE
#define CLONE(...) GET_CLONE(__VA_ARGS__, CLONE_6, CLONE_5, CLONE_4, CLONE_3, CLONE_2, CLONE_1, _0)(__VA_ARGS__)

// Node API Implementation Helpers:
//
// Provides a definition for everything in the Node API other than clone().
#define NODE_NO_CLONE(T) \
  void accept(Visitor* v) const override { \
    v->visit(this); \
  } \
  void accept(Editor* e) override { \
    e->edit(this); \
  } \
  T* accept(Builder* b) const override { \
    return (T*) b->build(this); \
  } \
  T* accept(Rewriter* r) override { \
    return (T*) r->rewrite(this); \
  }
// Provides a definition for everything in the Node API including clone().
#define NODE(T, ...) \
  CLONE(T, __VA_ARGS__) \
  NODE_NO_CLONE(T)

// Attribute API Helpers: 
//
// Value attributes are set and returned by reference.
#define VAL_GET_SET(T, t) \
  auto& get_##t() { \
    return PRIVATE(t); \
  } \
  const auto& get_##t() const { \
    return PRIVATE(t); \
  } \
  void set_##t(T rhs) { \
    PRIVATE(t) = rhs; \
  } \
  void swap_##t(T& rhs) { \
    std::swap(PRIVATE(t), rhs); \
  }
// Pointer attributes are set and returned by pointers. The pointer API
// guarantees that parent pointers are set when elements are attached and
// removed from this node. The pointer API also provides convenience methods
// for recursive invocations of clone and visitors.
#define PTR_GET_SET(T, t) \
  auto& get_##t() { \
    return PRIVATE(t); \
  } \
  const auto& get_##t() const { \
    return PRIVATE(t); \
  } \
  void set_##t(T rhs) { \
    assert(rhs != nullptr); \
    PRIVATE(t)->parent_ = nullptr; \
    PRIVATE(t) = rhs; \
    PRIVATE(t)->parent_ = this; \
  } \
  void replace_##t(T rhs) { \
    assert(rhs != nullptr); \
    delete PRIVATE(t); \
    PRIVATE(t) = rhs; \
    PRIVATE(t)->parent_ = this; \
  } \
  void swap_##t(T& rhs) { \
    assert(PRIVATE(t) != nullptr); \
    assert(rhs != nullptr); \
    std::swap(PRIVATE(t), rhs); \
    std::swap(PRIVATE(t)->parent_, rhs->parent_); \
  } \
  T clone_##t() const { \
    return PRIVATE(t)->clone(); \
  } \
  void accept_##t(Visitor* v) const { \
    PRIVATE(t)->accept(v); \
  } \
  void accept_##t(Visitor* v, std::function<void()> pre, std::function<void()> post) const { \
    pre(); \
    PRIVATE(t)->accept(v); \
    post(); \
  } \
  void accept_##t(Editor* e) { \
    PRIVATE(t)->accept(e); \
  } \
  T accept_##t(Builder* b) const { \
    return (T) PRIVATE(t)->accept(b); \
  } \
  T accept_##t(Rewriter* r) { \
    auto res = PRIVATE(t)->accept(r); \
    if (res != PRIVATE(t)) { \
      replace_##t(res); \
    } \
    return PRIVATE(t); \
  }
// Maybe attributes are handled similarly to pointer attributes. The only notable
// difference is an extended set of query operators and the absence of a swap
// method (in the case of a non-null/null pointer swap, it would be impossible
// to determine the non-null pointer's new parent). The maybe API also provides
// convenience methods for recursive invocations of clone and visitors.
#define MAYBE_GET_SET(T, t) \
  bool is_null_##t() const { \
    return PRIVATE(t) == nullptr; \
  } \
  bool is_non_null_##t() const { \
    return PRIVATE(t) != nullptr; \
  } \
  auto& get_##t() { \
    return PRIVATE(t); \
  } \
  const auto& get_##t() const { \
    return PRIVATE(t); \
  } \
  T remove_##t() { \
    assert(PRIVATE(t) != nullptr); \
    auto res = PRIVATE(t); \
    PRIVATE(t) = nullptr; \
    res->parent_ = nullptr; \
    return res; \
  } \
  void set_##t(T rhs) { \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->parent_ = nullptr; \
    } \
    PRIVATE(t) = rhs; \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->parent_ = this; \
    } \
  } \
  void replace_##t(T rhs) { \
    if (PRIVATE(t) != nullptr) { \
      delete PRIVATE(t); \
    } \
    PRIVATE(t) = rhs; \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->parent_ = this; \
    } \
  } \
  T clone_##t() const { \
    return (PRIVATE(t) != nullptr) ? PRIVATE(t)->clone() : nullptr; \
  } \
  void accept_##t(Visitor* v) const { \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->accept(v); \
    } \
  } \
  void accept_##t(Visitor* v, std::function<void()> pre, std::function<void()> post) const { \
    if (PRIVATE(t) != nullptr) { \
      pre(); \
      PRIVATE(t)->accept(v); \
      post(); \
    } \
  } \
  void accept_##t(Editor* e) { \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->accept(e); \
    } \
  } \
  T accept_##t(Builder* b) const { \
    return (PRIVATE(t) != nullptr) ? (T) PRIVATE(t)->accept(b) : nullptr; \
  } \
  T accept_##t(Rewriter* r) { \
    if (PRIVATE(t) != nullptr) { \
      auto res = PRIVATE(t)->accept(r); \
      if (res != PRIVATE(t)) { \
        replace_##t(res); \
      } \
    } \
    return PRIVATE(t); \
  }
// The many API provides the usual suite of iterator accessors in place of
// direct pointer access to the underlying container.  The many API also
// provides convenience methods for recursive invocations of clone and
// visitors.
#define MANY_GET_SET(T, t) \
  typedef typename Vector<T>::iterator iterator_##t; \
  typedef typename Vector<T>::const_iterator const_iterator_##t; \
  size_t size_##t() const { \
    return PRIVATE(t).size(); \
  } \
  bool empty_##t() const { \
    return PRIVATE(t).empty(); \
  } \
  iterator_##t begin_##t() { \
    return PRIVATE(t).begin(); \
  } \
  iterator_##t end_##t() { \
    return PRIVATE(t).end(); \
  } \
  const_iterator_##t begin_##t() const { \
    return PRIVATE(t).begin(); \
  } \
  const_iterator_##t end_##t() const { \
    return PRIVATE(t).end(); \
  } \
  T* get_##t(size_t n) { \
    return PRIVATE(t)[n]; \
  } \
  const T* get_##t(size_t n) const { \
    return PRIVATE(t)[n]; \
  } \
  T* front_##t() { \
    return PRIVATE(t).front(); \
  } \
  const T* front_##t() const { \
    return PRIVATE(t).front(); \
  } \
  T* back_##t() { \
    return PRIVATE(t).back(); \
  } \
  const T* back_##t() const { \
    return PRIVATE(t).back(); \
  } \
  void set_##t(size_t n, T* val) { \
    assert(val != nullptr); \
    PRIVATE(t)[n]->parent_ = nullptr; \
    PRIVATE(t)[n] = val; \
    val->parent_ = this; \
  } \
  void replace_##t(size_t n, T* val) { \
    assert(val != nullptr); \
    delete PRIVATE(t)[n]; \
    PRIVATE(t)[n] = val; \
    val->parent_ = this; \
  } \
  void conditional_replace_##t(size_t n, T* val) { \
    assert(val != nullptr); \
    if (val != PRIVATE(t)[n]) { \
      delete PRIVATE(t)[n]; \
      PRIVATE(t)[n] = val; \
      val->parent_ = this; \
    } \
  } \
  void push_front_##t(T* val) { \
    assert(val != nullptr); \
    val->parent_ = this; \
    PRIVATE(t).insert(PRIVATE(t).begin(), val); \
  } \
  void push_back_##t(T* val) { \
    assert(val != nullptr); \
    val->parent_ = this; \
    PRIVATE(t).push_back(val); \
  } \
  void pop_front_##t() { \
    assert(!PRIVATE(t).empty()); \
    PRIVATE(t).front()->parent_ = nullptr; \
    PRIVATE(t).erase(PRIVATE(t).begin()); \
  } \
  void pop_back_##t() { \
    assert(!PRIVATE(t).empty()); \
    PRIVATE(t).back()->parent_ = nullptr; \
    PRIVATE(t).pop_back(); \
  } \
  T* remove_front_##t() { \
    auto res = front_##t(); \
    pop_front_##t(); \
    return res; \
  } \
  T* remove_back_##t() { \
    auto res = back_##t(); \
    pop_back_##t(); \
    return res; \
  } \
  void clear_##t() { \
    for (auto n : PRIVATE(t)) { \
      n->parent_ = nullptr; \
    } \
    clear_##t(); \
  } \
  iterator_##t purge_##t(iterator_##t itr) { \
    assert(itr != end()); \
    delete *itr; \
    return PRIVATE(t).erase(itr); \
  } \
  void purge_to_##t(size_t n) { \
    while (PRIVATE(T).size() > n) { \
      auto n = back_##t(); \
      pop_back_##t(); \
      delete n; \
    } \
  } \
  void purge_##t() { \
    purge_to_##t(0); \
  } \
  T clone_##t() const { \
  } \
  void accept_##t(Visitor* v) const { \
  } \
  void accept_##t(Visitor* v, std::function<void()> pre, std::function<void()> post) const { \
  } \
  void accept_##t(Editor* e) { \
  } \
  T accept_##t(Builder* b) const { \
  } \
  T accept_##t(Rewriter* r) { \
  }

// Attribute Declaration Helpers:
//
// These are provided for the sake of expressiveness only. Everything is mapped
// down on to PRIVATE.
#define VAL_ATTR(T, t) T PRIVATE(t)
#define MAYBE_ATTR(T, t) T PRIVATE(t)
#define MANY_ATTR(T, t) T PRIVATE(t)
#define PTR_ATTR(T, t) T PRIVATE(t)
#define DECORATION(T, t) T PRIVATE(t)

#define HIERARCHY_VISIBILITY \
  template <typename T> \
  friend class Many; \
  friend class ArgAssign; \
  friend class Attributes; \
  friend class AttrSpec; \
  friend class BlockStatement; \
  friend class CaseGenerateItem; \
  friend class CaseItem; \
  friend class Event; \
  friend class BinaryExpression; \
  friend class ConditionalExpression; \
  friend class Concatenation; \
  friend class Identifier; \
  friend class MultipleConcatenation; \
  friend class Number; \
  friend class String; \
  friend class RangeExpression; \
  friend class UnaryExpression; \
  friend class GenerateBlock; \
  friend class Id; \
  friend class IfGenerateClause; \
  friend class ModuleDeclaration; \
  friend class AlwaysConstruct; \
  friend class IfGenerateConstruct; \
  friend class CaseGenerateConstruct; \
  friend class LoopGenerateConstruct; \
  friend class InitialConstruct; \
  friend class ContinuousAssign; \
  friend class Declaration; \
  friend class GenvarDeclaration; \
  friend class IntegerDeclaration; \
  friend class LocalparamDeclaration; \
  friend class NetDeclaration; \
  friend class ParameterDeclaration; \
  friend class RegDeclaration; \
  friend class GenerateRegion; \
  friend class ModuleInstantiation; \
  friend class PortDeclaration; \
  friend class BlockingAssign; \
  friend class NonblockingAssign; \
  friend class ParBlock; \
  friend class SeqBlock; \
  friend class CaseStatement; \
  friend class ConditionalStatement; \
  friend class ForStatement; \
  friend class ForeverStatement; \
  friend class RepeatStatement; \
  friend class WhileStatement; \
  friend class TimingControlStatement; \
  friend class DisplayStatement; \
  friend class FinishStatement; \
  friend class WriteStatement; \
  friend class WaitStatement; \
  friend class DelayControl; \
  friend class EventControl; \
  friend class VariableAssign

#endif
