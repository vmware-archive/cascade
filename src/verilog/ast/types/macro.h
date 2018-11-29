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
// Binds a leaf (non-pointer attribute) to an input argument.
#define LEAF_SETUP(t) \
  PRIVATE(t) = INPUT(t);
// Binds a tree (pointer) attribute to an input argument. Throws an assertion
// for null pointers and sets the argument's parent pointer to the node it's
// being attached to.
#define TREE_SETUP(t) \
  assert(INPUT(t) != nullptr); \
  PRIVATE(t) = INPUT(t); \
  PRIVATE(t)->parent_ = this;
// Binds a maybe attribute to an input argument. Identical to tree setup, but
// allows for null pointers.
#define MAYBE_SETUP(t) \
  PRIVATE(t) = INPUT(t); \
  if (PRIVATE(t) != nullptr) { \
    PRIVATE(t)->parent_ = this; \
  }

// Destructor Helpers:
//
// Frees memory associated with a leaf (non-pointer) attribute (does nothing).
#define LEAF_TEARDOWN(t)
// Frees memory associated with a tree (pointer) attribute by calling delete.
// Throws an assertion of this attribute is a null pointer.
#define TREE_TEARDOWN(t) \
  assert(PRIVATE(t) != nullptr); \
  delete PRIVATE(t);
// Frees memory associated with a maybe attribute. Identical to tree teardown
// but allows for null pointers.
#define MAYBE_TEARDOWN(t) \
  if (PRIVATE(t) != nullptr) { \
    delete PRIVATE(t); \
  }

// Clone Implementation Helpers:
// 
// Leaf attributes are passed by value. 
#define LEAF(x) PRIVATE(x)
// Tree attributes are cloned before being passed as arguments.
#define TREE(x) PRIVATE(x)->clone()
// Maybe attribtues are only cloned if they are non-null.
#define MAYBE(x) (PRIVATE(x) != nullptr) ? PRIVATE(x)->clone() : nullptr
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

// Getter Implementation Helpers: 
//
// Leaf attributes are returned by const reference
#define LEAF_GET(t) \
  auto& get_##t() { \
    return PRIVATE(t); \
  } \
  const auto& get_##t() const { \
    return PRIVATE(t); \
  }
// Tree attributes are returned by const reference as well
#define TREE_GET(t) \
  auto& get_##t() { \
    return PRIVATE(t); \
  } \
  const auto& get_##t() const { \
    return PRIVATE(t); \
  }
// Returns an attribute by const reference. Note that this can return a null
// pointer.
#define MAYBE_GET(t) \
  auto& get_##t() { \
    return PRIVATE(t); \
  } \
  const auto& get_##t() const { \
    return PRIVATE(t); \
  }

// Setter Implementation Helpers:
//
// Leaf attributes are written over unconditionally. 
#define LEAF_SET(t) \
  template <typename T> \
  void set_##t(T rhs) { \
    PRIVATE(t) = rhs; \
  } \
  template <typename T> \
  void replace_##t(T rhs) { \
    PRIVATE(t) = rhs; \
  } \
  template <typename T> \
  void conditional_replace_##t(T rhs) { \
    PRIVATE(t) = rhs; \
  } \
  template <typename T> \
  void swap_##t(T& rhs) { \
    std::swap(PRIVATE(t), rhs); \
  }
// Tree attributes are written over and have their parent pointers updated
// accordingly.  Throws an assertion if either argument is a null pointer.
#define TREE_SET(t) \
  template <typename T> \
  void set_##t(T rhs) { \
    assert(PRIVATE(t) != nullptr); \
    assert(rhs != nullptr); \
    PRIVATE(t)->parent_ = nullptr; \
    PRIVATE(t) = rhs; \
    PRIVATE(t)->parent_ = this; \
  } \
  template <typename T> \
  void replace_##t(T rhs) { \
    assert(PRIVATE(t) != nullptr); \
    assert(rhs != nullptr); \
    delete PRIVATE(t); \
    PRIVATE(t) = rhs; \
    PRIVATE(t)->parent_ = this; \
  } \
  template <typename T> \
  void conditional_replace_##t(T rhs) { \
    assert(PRIVATE(t) != nullptr); \
    assert(rhs != nullptr); \
    if (PRIVATE(t) == rhs) { \
      return; \
    } \
    delete PRIVATE(t); \
    PRIVATE(t) = rhs; \
    PRIVATE(t)->parent_ = this; \
  } \
  template <typename T> \
  void swap_##t(T& rhs) { \
    assert(PRIVATE(t) != nullptr); \
    assert(rhs != nullptr); \
    std::swap(PRIVATE(t), rhs); \
    std::swap(PRIVATE(t)->parent_, rhs->parent_); \
  }
// Maybe attributes are written over and have their parent pointers updated
// accordingly, but both left and right hand sides are permitted to be null
// pointers. Swapping is not permitted. If we attempted to swap a pointer with
// a null pointer, we'd have no way of knowing where its new parent was.
#define MAYBE_SET(t) \
  template <typename T> \
  void set_##t(T rhs) { \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->parent_ = nullptr; \
    } \
    PRIVATE(t) = rhs; \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->parent_ = this; \
    } \
  } \
  template <typename T> \
  void replace_##t(T rhs) { \
    if (PRIVATE(t) != nullptr) { \
      delete PRIVATE(t); \
    } \
    PRIVATE(t) = rhs; \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->parent_ = this; \
    } \
  } \
  template <typename T> \
  void conditional_replace_##t(T rhs) { \
    if (PRIVATE(t) == rhs) { \
      return; \
    } \
    if (PRIVATE(t) != nullptr) { \
      delete PRIVATE(t); \
    } \
    PRIVATE(t) = rhs; \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->parent_ = this; \
    } \
  } \

// Extended Maybe definiton helpers
#define MAYBE_HELPERS(T, t) \
  bool is_null_##t() const { \
    return PRIVATE(t) == nullptr; \
  } \
  bool is_non_null_##t() const { \
    return PRIVATE(t) != nullptr; \
  } \
  void maybe_accept_##t(Visitor* v) const { \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->accept(v); \
    } \
  } \
  void maybe_accept_##t(Editor* e) { \
    if (PRIVATE(t) != nullptr) { \
      PRIVATE(t)->accept(e); \
    } \
  } \
  T maybe_accept_##t(Builder* b) const { \
    return (PRIVATE(t) != nullptr) ? (T) PRIVATE(t)->accept(b) : nullptr; \
  } \
  T maybe_accept_##t(Rewriter* r) { \
    return (PRIVATE(t) != nullptr) ? (T) PRIVATE(t)->accept(r) : nullptr; \
  }

// Convenience Definitions:
//
// Inserts definitions for get and set
#define LEAF_GET_SET(t) \
  LEAF_GET(t) \
  LEAF_SET(t)
#define TREE_GET_SET(t) \
  TREE_GET(t) \
  TREE_SET(t)
#define MAYBE_GET_SET(T, t) \
  MAYBE_HELPERS(T, t) \
  MAYBE_GET(t) \
  MAYBE_SET(t)

// Attribute Declaration Helpers:
//
// These are provided for the sake of expressiveness only. Everything is mapped
// down on to PRIVATE.
#define LEAF_ATTR(T, t) T PRIVATE(t)
#define MAYBE_ATTR(T, t) T PRIVATE(t)
#define TREE_ATTR(T, t) T PRIVATE(t)
#define DECORATION(T, t) T PRIVATE(t)

#define HIERARCHY_VISIBILITY \
  template <typename T> \
  friend class Many; \
  template <typename T> \
  friend class Maybe; \
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
