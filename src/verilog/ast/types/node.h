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

#ifndef CASCADE_SRC_VERILOG_AST_NODE_H
#define CASCADE_SRC_VERILOG_AST_NODE_H

#include "verilog/ast/types/macro.h"
#include "verilog/ast/visitors/builder.h"
#include "verilog/ast/visitors/editor.h"
#include "verilog/ast/visitors/rewriter.h"
#include "verilog/ast/visitors/visitor.h"

namespace cascade {

class Node {
  public:
    // Type tags:
    enum class Tag : uint32_t {
      // Tags for abstract classes are assigned in most- to least-significant
      // bit order. Tags are assigned such that we have the invariant:
      // TAG(superclass) & TAG(subclass) == TAG(superclass).
      node                           = 0b1'0000'0000'0'0'00000'0000000000000000,
      expression                     = 0b1'1000'0000'0'0'00000'0000000000000000,
      primary                        = 0b1'1000'1000'0'0'00000'0000000000000000,
      module_item                    = 0b1'0100'0000'0'0'00000'0000000000000000,
      construct                      = 0b1'0100'1000'0'0'00000'0000000000000000,
      generate_construct             = 0b1'0100'1000'1'0'00000'0000000000000000,
      conditional_generate_construct = 0b1'0100'1000'1'1'00000'0000000000000000,
      declaration                    = 0b1'0100'0100'0'0'00000'0000000000000000,
      instantiation                  = 0b1'0100'0010'0'0'00000'0000000000000000,
      statement                      = 0b1'0010'0000'0'0'00000'0000000000000000,
      assign_statement               = 0b1'0010'1000'0'0'00000'0000000000000000,
      block_statement                = 0b1'0010'0100'0'0'00000'0000000000000000,
      loop_statement                 = 0b1'0010'0010'0'0'00000'0000000000000000,
      system_task_enable_statement   = 0b1'0010'0001'0'0'00000'0000000000000000,
      timing_control                 = 0b1'0001'0000'0'0'00000'0000000000000000,
      // Tags for supporting concepts are assigned in most- to least- significat
      // bit order following bits reserved for abstract classes.
      scope                          = 0b0'0000'0000'0'0'10000'0000000000000000,
      // Tags for concrete classes are assigned in ascending order from zero
      // and unioned with the tag for their most-specific subclass and any
      // supporting concepts.
      arg_assign                     =  0 | node,
      attributes                     =  1 | node,
      attr_spec                      =  2 | node,
      case_generate_item             =  3 | node,
      case_item                      =  4 | node,
      event                          =  5 | node,
      binary_expression              =  6 | expression,
      conditional_expression         =  7 | expression,
      feof_expression                =  8 | expression,
      fopen_expression               =  9 | expression,
      concatenation                  = 10 | primary, 
      identifier                     = 11 | primary, 
      multiple_concatenation         = 12 | primary, 
      number                         = 13 | primary, 
      string                         = 14 | primary, 
      range_expression               = 15 | expression, 
      unary_expression               = 16 | expression, 
      generate_block                 = 17 | scope | node, 
      id                             = 18 | node, 
      if_generate_clause             = 19 | node, 
      module_declaration             = 20 | scope | node, 
      always_construct               = 21 | construct, 
      case_generate_construct        = 22 | conditional_generate_construct, 
      if_generate_construct          = 23 | conditional_generate_construct, 
      loop_generate_construct        = 24 | generate_construct, 
      initial_construct              = 25 | construct, 
      continuous_assign              = 26 | module_item, 
      genvar_declaration             = 27 | declaration, 
      integer_declaration            = 28 | declaration, 
      localparam_declaration         = 29 | declaration, 
      net_declaration                = 30 | declaration, 
      parameter_declaration          = 31 | declaration, 
      reg_declaration                = 32 | declaration, 
      generate_region                = 33 | module_item, 
      module_instantiation           = 34 | instantiation, 
      port_declaration               = 35 | module_item, 
      simple_id                      = 36 | node, 
      blocking_assign                = 37 | assign_statement, 
      nonblocking_assign             = 38 | assign_statement, 
      par_block                      = 39 | scope | block_statement, 
      seq_block                      = 40 | scope | block_statement, 
      case_statement                 = 41 | statement, 
      conditional_statement          = 42 | statement, 
      for_statement                  = 43 | loop_statement, 
      forever_statement              = 44 | loop_statement, 
      repeat_statement               = 45 | loop_statement, 
      while_statement                = 46 | loop_statement, 
      timing_control_statement       = 47 | statement, 
      finish_statement               = 48 | system_task_enable_statement, 
      fseek_statement                = 49 | system_task_enable_statement, 
      get_statement                  = 50 | system_task_enable_statement, 
      put_statement                  = 51 | system_task_enable_statement, 
      puts_statement                 = 52 | system_task_enable_statement, 
      restart_statement              = 53 | system_task_enable_statement, 
      retarget_statement             = 54 | system_task_enable_statement, 
      save_statement                 = 55 | system_task_enable_statement, 
      wait_statement                 = 56 | statement, 
      delay_control                  = 57 | timing_control,
      event_control                  = 58 | timing_control,
      variable_assign                = 59 | node
    };

    // Constructors:
    Node(Tag tag);
    virtual ~Node() = default;

    // Node Interface:
    virtual Node* clone() const = 0;
    virtual void accept(Visitor* v) const = 0;
    virtual void accept(Editor* e) = 0;
    virtual Node* accept(Builder* b) const = 0;
    virtual Node* accept(Rewriter* r) = 0;

    // Get/Set:
    Node* get_parent();
    const Node* get_parent() const;

    // Runtime Type Identification:
    Tag get_tag() const;
    bool is_concept(Tag tag) const;
    bool is_subclass_of(Tag tag) const;
    bool is(Tag tag) const;

  private:
    friend class Elaborate;
    friend class Inline;
    HIERARCHY_VISIBILITY;
    DECORATION(Node*, parent);

    friend class Evaluate;
    friend class SwLogic;
    DECORATION(uint32_t, common);
    // common_[0]    Evaluate: needs_update_
    // common_[1]    SwLogic:  active_
    // common_[2-4]  Number:   format_
    // common_[5]    Number:   signed_
    // common_[6-31] Number:   size_

    DECORATION(Tag, tag);

    template <size_t idx>
    void set_flag(bool b);
    template <size_t idx>
    bool get_flag() const;

    template <size_t idx, size_t w>
    void set_val(uint32_t val);
    template <size_t idx, size_t w>
    uint32_t get_val() const;
};

inline Node::Node(Tag tag) {
  set_flag<0>(true);
  set_flag<1>(false);
  tag_ = tag;
}

inline Node* Node::get_parent() {
  return parent_;
}

inline const Node* Node::get_parent() const {
  return parent_;
}

inline Node::Tag Node::get_tag() const {
  return tag_;
}

inline bool Node::is_concept(Tag tag) const {
  return (static_cast<uint32_t>(tag_) & static_cast<uint32_t>(tag)) == static_cast<uint32_t>(tag);
}

inline bool Node::is_subclass_of(Tag tag) const {
  return (static_cast<uint32_t>(tag_) & static_cast<uint32_t>(tag)) == static_cast<uint32_t>(tag);
}

inline bool Node::is(Tag tag) const {
  return tag_ == tag;
}

template <size_t idx>
inline void Node::set_flag(bool b) {
  if (b) {
    common_ |= (static_cast<uint32_t>(1) << idx);
  } else {
    common_ &= ~(static_cast<uint32_t>(1) << idx);
  }
}

template <size_t idx>
inline bool Node::get_flag() const {
  return common_ & (static_cast<uint32_t>(1) << idx);
}

template <size_t idx, size_t w>
inline void Node::set_val(uint32_t val) {
  const auto mask = (static_cast<uint32_t>(1) << w) - 1;
  val &= mask;
  common_ &= ~(mask << idx);
  common_ |= (val << idx);
}

template <size_t idx, size_t w>
inline uint32_t Node::get_val() const {
  const auto mask = (static_cast<uint32_t>(1) << w) - 1;
  return static_cast<uint32_t>((common_ >> idx) & mask);
}

} // namespace cascade

#endif
