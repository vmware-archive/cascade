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

#ifndef CASCADE_SRC_VERILOG_AST_TYPES_MACRO_H
#define CASCADE_SRC_VERILOG_AST_TYPES_MACRO_H

#include <cassert>
#include <functional>
#include <iterator>
#include "src/base/container/vector.h"

// Naming Helpers:
//
// Calling convention for value arguments. Variable names are suffixed with
// two trailing underscores.
#define INPUT(x) x##__
// Calling convention for iterator arguments. Variable names are suffixed with
// begin and end and then two trailing underscores 
#define BEGIN_INPUT(x) x##_begin__
#define END_INPUT(x) x##_end__
// Naming convention for private members. Attribtues are suffixed with a single
// trailing underscore.
#define PRIVATE(x) x##_

// Binds a value attribute to an input argument.
#define VAL_SETUP(t) \
  PRIVATE(t) = INPUT(t);
// Binds a pointer attribute to an input argument. Throws an assertion
// for null pointers and sets the argument's parent pointer to this node.
#define PTR_SETUP(t) \
  assert(INPUT(t) != nullptr); \
  PRIVATE(t) = INPUT(t); \
  PRIVATE(t)->parent_ = this;
// Binds a maybe attribute to nullptr.
#define MAYBE_DEFAULT_SETUP(t) \
  PRIVATE(t) = nullptr;
// Binds a maybe attribute to an input argument. Identical to pointer setup, but
// allows for null pointers.
#define MAYBE_SETUP(t) \
  PRIVATE(t) = INPUT(t); \
  if (PRIVATE(t) != nullptr) { \
    PRIVATE(t)->parent_ = this; \
  }
// Does nothing. Leaves a many argument alone.
#define MANY_DEFAULT_SETUP(t) 
// Copies the elements in an iterator range into a many object.  Sets the
// parent pointer for every element to this node.
#define MANY_SETUP(t) \
  for (auto itr = BEGIN_INPUT(t); itr != END_INPUT(t); ++itr) { \
    push_back_##t(*itr); \
  } \

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

// Node API Implementation Helpers:
//
// Provides a definition for everything in the Node API other than clone().
#define NODE(T) \
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
// Clones a non-null maybe and inserts the result into the corresponding
// location in res
#define MAYBE_CLONE(t) \
  if (PRIVATE(t) != nullptr) { \
    res->set_##t(PRIVATE(t)->clone()); \
  } 
// Clones the elements in a many and inserts them into the corresponding
// location in res
#define MANY_CLONE(t) \
  for (auto n : PRIVATE(t)) { \
    res->push_back_##t(n->clone()); \
  } 

// Attribute API Helpers: 
//
// Value attributes are set and returned by reference.
#define VAL_GET_SET(C, T, t) \
  T get_##t() { \
    return PRIVATE(t); \
  } \
  T get_##t() const { \
    return PRIVATE(t); \
  } \
  void set_##t(T rhs) { \
    PRIVATE(t) = rhs; \
  } 
// Pointer attributes are set and returned by pointers. The pointer API
// guarantees that parent pointers are set when elements are attached and
// removed from this node. The pointer API also provides convenience methods
// for recursive invocations of clone and visitors.
#define PTR_GET_SET(C, T, t) \
  T* get_##t() { \
    return PRIVATE(t); \
  } \
  const T* get_##t() const { \
    return PRIVATE(t); \
  } \
  void set_##t(T* rhs) { \
    assert(rhs != nullptr); \
    PRIVATE(t)->parent_ = nullptr; \
    PRIVATE(t) = rhs; \
    PRIVATE(t)->parent_ = this; \
  } \
  void replace_##t(T* rhs) { \
    assert(rhs != nullptr); \
    delete PRIVATE(t); \
    PRIVATE(t) = rhs; \
    PRIVATE(t)->parent_ = this; \
  } \
  template <typename N> \
  void swap_##t(N* n) { \
    auto temp = n->get_##t(); \
    assert(temp != nullptr); \
    n->set_##t(PRIVATE(t)); \
    PRIVATE(t) = temp; \
    PRIVATE(t)->parent_ = this; \
  } \
  T* clone_##t() const { \
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
  T* accept_##t(Builder* b) const { \
    auto res = PRIVATE(t)->accept(b); \
    assert(res != nullptr); \
    return res; \
  } \
  T* accept_##t(Rewriter* r) { \
    auto res = PRIVATE(t)->accept(r); \
    if (res != PRIVATE(t)) { \
      replace_##t(res); \
    } \
    return PRIVATE(t); \
  }
// Maybe attributes are handled similarly to pointer attributes.  The maybe API
// also provides convenience methods for recursive invocations of clone and
// visitors.
#define MAYBE_GET_SET(C, T, t) \
  bool is_null_##t() const { \
    return PRIVATE(t) == nullptr; \
  } \
  bool is_non_null_##t() const { \
    return PRIVATE(t) != nullptr; \
  } \
  T* get_##t() { \
    return PRIVATE(t); \
  } \
  const T* get_##t() const { \
    return PRIVATE(t); \
  } \
  T* remove_##t() { \
    assert(PRIVATE(t) != nullptr); \
    auto res = PRIVATE(t); \
    res->parent_ = nullptr; \
    PRIVATE(t) = nullptr; \
    return res; \
  } \
  void set_##t(T* rhs) { \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->parent_ = nullptr; \
    } \
    PRIVATE(t) = rhs; \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->parent_ = this; \
    } \
  } \
  void replace_##t(T* rhs) { \
    if (PRIVATE(t) != nullptr) { \
      delete PRIVATE(t); \
    } \
    PRIVATE(t) = rhs; \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->parent_ = this; \
    } \
  } \
  T* clone_##t() const { \
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
  T* accept_##t(Builder* b) const { \
    return (PRIVATE(t) != nullptr) ? PRIVATE(t)->accept(b) : nullptr; \
  } \
  T* accept_##t(Rewriter* r) { \
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
#define MANY_GET_SET(C, T, t) \
  class iterator_##t { \
    public: \
      typedef typename Vector<T*>::difference_type difference_type; \
      typedef T* value_type; \
      typedef T* const* pointer; \
      typedef T* reference; \
      typedef std::forward_iterator_tag iterator_category; \
      explicit iterator_##t(typename Vector<T*>::iterator itr) : itr_(itr) { } \
      reference operator*() { return *itr_; } \
      pointer operator->() { return itr_; } \
      bool operator==(const iterator_##t& rhs) const { return itr_ == rhs.itr_; } \
      bool operator!=(const iterator_##t& rhs) const { return itr_ != rhs.itr_; } \
      iterator_##t& operator++() { ++itr_; return *this; } \
      iterator_##t operator++(int) { auto temp = *this; ++itr_; return temp; } \
      iterator_##t operator+(size_t n) { return iterator_##t(itr_+n); } \
    private: \
      friend class C; \
      typename Vector<T*>::iterator itr_; \
  }; \
  class const_iterator_##t { \
    public: \
      typedef typename Vector<T*>::difference_type difference_type; \
      typedef const T* value_type; \
      typedef const T* const* pointer; \
      typedef const T* reference; \
      typedef std::forward_iterator_tag iterator_category; \
      explicit const_iterator_##t(typename Vector<T*>::const_iterator itr) : itr_(itr) { } \
      reference operator*() { return *itr_; } \
      pointer operator->() { return itr_; } \
      bool operator==(const const_iterator_##t& rhs) const { return itr_ == rhs.itr_; } \
      bool operator!=(const const_iterator_##t& rhs) const { return itr_ != rhs.itr_; } \
      const_iterator_##t& operator++() { ++itr_; return *this; } \
      const_iterator_##t operator++(int) { auto temp = *this; ++itr_; return temp; } \
      const_iterator_##t operator+(size_t n) { return const_iterator_##t(itr_+n); } \
    private: \
      friend class C; \
      typename Vector<T*>::const_iterator itr_; \
  }; \
  class back_insert_iterator_##t { \
    public: \
      typedef void difference_type; \
      typedef void value_type; \
      typedef void pointer; \
      typedef void reference; \
      typedef std::output_iterator_tag iterator_category; \
      explicit back_insert_iterator_##t(C* c) : c_(c) { } \
      back_insert_iterator_##t& operator=(T* val) { c_->push_back_##t(val); return *this; } \
    private: \
      C* c_; \
  }; \
  size_t size_##t() const { \
    return PRIVATE(t).size(); \
  } \
  bool empty_##t() const { \
    return PRIVATE(t).empty(); \
  } \
  iterator_##t begin_##t() { \
    return iterator_##t(PRIVATE(t).begin()); \
  } \
  iterator_##t end_##t() { \
    return iterator_##t(PRIVATE(t).end()); \
  } \
  const_iterator_##t begin_##t() const { \
    return const_iterator_##t(PRIVATE(t).begin()); \
  } \
  const_iterator_##t end_##t() const { \
    return const_iterator_##t(PRIVATE(t).end()); \
  } \
  back_insert_iterator_##t back_inserter_##t() { \
    return back_insert_iterator_##t(this); \
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
  template <typename InputItr> \
  void push_back_##t(InputItr begin, InputItr end) { \
    for (; begin != end; ++begin) { \
      push_back_##t(*begin); \
    } \
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
    PRIVATE(t).clear(); \
  } \
  iterator_##t purge_##t(iterator_##t itr) { \
    assert(itr != end_##t()); \
    delete *itr; \
    return iterator_##t(PRIVATE(t).erase(itr.itr_)); \
  } \
  void purge_to_##t(size_t n) { \
    while (size_##t() > n) { \
      delete remove_back_##t(); \
    } \
  } \
  void purge_##t() { \
    purge_to_##t(0); \
  } \
  void accept_##t(Visitor* v) const { \
    for (const auto n : PRIVATE(t)) { \
      n->accept(v); \
    } \
  } \
  template <typename OutputItr> \
  void clone_##t(OutputItr itr) const { \
    for (const auto n : PRIVATE(t)) { \
      itr = n->clone(); \
    } \
  } \
  void accept_##t(Visitor* v, std::function<void()> pre, std::function<void()> post) const { \
    for (const auto n : PRIVATE(t)) { \
      pre(); \
      n->accept(v); \
      post(); \
    } \
  } \
  void accept_##t(Editor* e) { \
    for (auto n : PRIVATE(t)) { \
      n->accept(e); \
    } \
  } \
  template <typename OutputItr> \
  void accept_##t(Builder* b, OutputItr itr) const { \
    for (const auto n : PRIVATE(t)) { \
      if (auto res = n->accept(b)) { \
        itr = res; \
      } \
    } \
  } \
  void accept_##t(Rewriter* r) { \
    for (auto& n : PRIVATE(t)) { \
      auto res = n->accept(r); \
      if (n != res) { \
        delete n; \
        n = res; \
        n->parent_ = this; \
      } \
    } \
  }

// Attribute Declaration Helpers:
//
#define VAL_ATTR(T, t) T PRIVATE(t)
#define PTR_ATTR(T, t) T* PRIVATE(t)
#define MAYBE_ATTR(T, t) T* PRIVATE(t)
#define MANY_ATTR(T, t) Vector<T*> PRIVATE(t)
#define DECORATION(T, t) T PRIVATE(t)

#define HIERARCHY_VISIBILITY \
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
