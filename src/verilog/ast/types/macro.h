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
#define INPUT(x) x##__
#define PRIVATE(x) x##_

// Constructors:
#define LEAF_SETUP(t) \
  PRIVATE(t) = INPUT(t);
#define LEAF_TEARDOWN(t) \
  // Does nothing.
#define TREE_SETUP(t) \
  assert(INPUT(t) != nullptr); \
  PRIVATE(t) = INPUT(t); \
  PRIVATE(t)->parent_ = this;
#define TREE_TEARDOWN(t) \
  assert(PRIVATE(t) != nullptr); \
  delete PRIVATE(t);

// Node Interface:
#define LEAF(x) PRIVATE(x)
#define TREE(x) PRIVATE(x)->clone()
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
#define GET_CLONE(_0, _1, _2, _3, _4, _5, CLONE, ...) CLONE
#define CLONE(...) GET_CLONE(__VA_ARGS__, CLONE_5, CLONE_4, CLONE_3, CLONE_2, CLONE_1, _0)(__VA_ARGS__)
#define NODE(T, ...) \
  CLONE(T, __VA_ARGS__) \
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

// Attribute Getters and Setters:
#define GET(t) \
  auto& get_##t() { \
    return PRIVATE(t); \
  } \
  const auto& get_##t() const { \
    return PRIVATE(t); \
  }
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
    if (PRIVATE(t) != rhs) { \
      replace_##t(rhs); \
    } \
  } \
  template <typename T> \
  void swap_##t(T& rhs) { \
    std::swap(PRIVATE(t), rhs); \
  }
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
    if (PRIVATE(t) != rhs) { \
      replace_##t(rhs); \
    } \
  } \
  template <typename T> \
  void swap_##t(T& rhs) { \
    assert(PRIVATE(t) != nullptr); \
    assert(rhs != nullptr); \
    std::swap(PRIVATE(t), rhs); \
    std::swap(PRIVATE(t)->parent_, rhs->parent_); \
  }
#define LEAF_GET_SET(t) \
  GET(t) \
  LEAF_SET(t)
#define TREE_GET_SET(t) \
  GET(t) \
  TREE_SET(t)

// Attribute Declaration
#define LEAF_ATTR(T, t) T PRIVATE(t)
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
  friend class NestedExpression; \
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
