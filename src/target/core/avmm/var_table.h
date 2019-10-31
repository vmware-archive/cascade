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

#ifndef CASCADE_SRC_TARGET_CORE_AVMM_VAR_TABLE_H
#define CASCADE_SRC_TARGET_CORE_AVMM_VAR_TABLE_H

#include <cassert>
#include <functional>
#include <unordered_map>
#include "common/bits.h"
#include "common/vector.h"
#include "verilog/analyze/evaluate.h"
#include "verilog/analyze/resolve.h"
#include "verilog/ast/ast.h"

namespace cascade::avmm {

template <typename T>
class VarTable {
  public:
    // Row Type:
    struct Row {
      size_t begin;
      size_t elements;
      size_t bits_per_element;
      size_t words_per_element;
    };

    // IO Typedefs:
    typedef std::function<T(size_t)> Read;
    typedef std::function<void(size_t, T)> Write;

    // Iterator Typedefs:
    typedef typename std::unordered_map<const Identifier*, const Row>::const_iterator const_var_iterator;
    typedef typename std::unordered_map<const Expression*, const Row>::const_iterator const_expr_iterator;
        
    // Constructors:
    VarTable(Read read, Write write);

    // Inserts an element into the table.
    void insert_var(const Identifier* id);
    // Inserts an expression into the table.
    void insert_expr(const Expression* e);
    // Returns the number of words in the variable segment of the var table.
    // This includes control variables (eg <there_are_updates>).
    size_t var_size() const;
    // Returns the number of words in the expression segment of the var table.
    size_t expr_size() const;
    // Returns the total number of entries in both variable and expression segments 
    // of the var table.
    size_t size() const;

    // Returns a pointer to an element in the table or end on failure
    const_var_iterator var_find(const Identifier* id) const;
    // Returns a pointer to the beginning of the table
    const_var_iterator var_begin() const;
    // Returns a pointer ot the end of the table
    const_var_iterator var_end() const;

    // Returns a pointer to an element in the table or end on failure
    const_expr_iterator expr_find(const Expression* e) const;
    // Returns a pointer to the beginning of the table
    const_expr_iterator expr_begin() const;
    // Returns a pointer ot the end of the table
    const_expr_iterator expr_end() const;


    // Returns the starting index of this identifier.
    size_t index(const Identifier* id) const;
    // Returns the address of the there_are_updates control variable.
    size_t there_are_updates_index() const;
    // Returns the address of the apply_update control variable.
    size_t apply_update_index() const;
    // Returns the address of the drop_update control variable.
    size_t drop_update_index() const;
    // Returns the address of the task control variable.
    size_t there_were_tasks_index() const;
    // Returns the address of the resume control variable.
    size_t resume_index() const;
    // Returns the address of the reset control variable.
    size_t reset_index() const;
    // Returns the address of the done control variable.
    size_t done_index() const;
    // Returns the address of the open_loop control variable.
    size_t open_loop_index() const;
    // Reserved for debugging
    size_t debug_index() const;

    // Reads the value of a control variable
    T read_control_var(size_t index) const;
    // Writes the value of a control variable
    void write_control_var(size_t index, T val);

    // Reads the value of a variable
    void read_var(const Identifier* id) const; 
    // Writes the value of a scalar variable
    void write_var(const Identifier* id, const Bits& val);
    // Writes the value of an array variable
    void write_var(const Identifier* id, const Vector<Bits>& val);

    // Reads the value of an expression
    T read_expr(const Expression* e) const;
    // Writes the value of an expression
    void write_expr(const Expression* e, T val);

  private:
    Read read_;
    Write write_;

    size_t next_index_;
    std::unordered_map<const Identifier*, const Row> vtable_;
    std::unordered_map<const Expression*, const Row> etable_;

    constexpr size_t bits_per_word() const;
};

using VarTable16 = VarTable<uint16_t>;
using VarTable32 = VarTable<uint32_t>;
using VarTable64 = VarTable<uint64_t>;

template <typename T>
inline VarTable<T>::VarTable(Read read, Write write) {
  read_ = read;
  write_ = write;
  next_index_ = 0;
}

template <typename T>
inline void VarTable<T>::insert_var(const Identifier* id) {
  assert(var_find(id) == var_end());

  Row row;
  row.begin = next_index_;
  row.elements = 1;
  for (auto d : Evaluate().get_arity(id)) {
    row.elements *= d;
  }
  const auto* r = Resolve().get_resolution(id);
  assert(r != nullptr);
  row.bits_per_element = std::max(Evaluate().get_width(r), Evaluate().get_width(id));
  row.words_per_element = (row.bits_per_element + bits_per_word() - 1) / bits_per_word();

  vtable_.insert(std::make_pair(id, row));
  next_index_ += (row.elements * row.words_per_element);
}

template <typename T>
inline void VarTable<T>::insert_expr(const Expression* e) {
  assert(expr_find(e) == expr_end());

  Row row;
  row.begin = expr_size();
  row.elements = 1;
  row.bits_per_element = Evaluate().get_width(e);
  row.words_per_element = (row.bits_per_element + bits_per_word() - 1) / bits_per_word();

  etable_.insert(std::make_pair(e, row));
}

template <typename T>
inline size_t VarTable<T>::var_size() const {
  return debug_index() + 1;
}

template <typename T>
inline size_t VarTable<T>::expr_size() const {
  return etable_.size();
}

template <typename T>
inline size_t VarTable<T>::size() const {
  return var_size() + expr_size();
}

template <typename T>
inline typename VarTable<T>::const_var_iterator VarTable<T>::var_find(const Identifier* id) const {
  return vtable_.find(id);
}

template <typename T>
inline typename VarTable<T>::const_var_iterator VarTable<T>::var_begin() const {
  return vtable_.begin();
}

template <typename T>
inline typename VarTable<T>::const_var_iterator VarTable<T>::var_end() const {
  return vtable_.end();
}

template <typename T>
inline typename VarTable<T>::const_expr_iterator VarTable<T>::expr_find(const Expression* e) const {
  return etable_.find(e);
}

template <typename T>
inline typename VarTable<T>::const_expr_iterator VarTable<T>::expr_begin() const {
  return etable_.begin();
}

template <typename T>
inline typename VarTable<T>::const_expr_iterator VarTable<T>::expr_end() const {
  return etable_.end();
}

template <typename T>
inline size_t VarTable<T>::index(const Identifier* id) const {
  const auto itr = vtable_.find(id);
  assert(itr != vtable_.end());
  return itr->second.index_;
}

template <typename T>
inline size_t VarTable<T>::there_are_updates_index() const {
  return next_index_;
}

template <typename T>
inline size_t VarTable<T>::apply_update_index() const {
  return next_index_ + 1;
}

template <typename T>
inline size_t VarTable<T>::drop_update_index() const {
  return next_index_ + 2;
}

template <typename T>
inline size_t VarTable<T>::there_were_tasks_index() const {
  return next_index_ + 3;
}

template <typename T>
inline size_t VarTable<T>::resume_index() const {
  return next_index_ + 4;
}

template <typename T>
inline size_t VarTable<T>::reset_index() const {
  return next_index_ + 5;
}

template <typename T>
inline size_t VarTable<T>::done_index() const {
  return next_index_ + 6;
}

template <typename T>
inline size_t VarTable<T>::open_loop_index() const {
  return next_index_ + 7;
}

template <typename T>
inline size_t VarTable<T>::debug_index() const {
  return next_index_ + 8;
}

template <typename T>
inline T VarTable<T>::read_control_var(size_t index) const {
  assert(index >= there_are_updates_index());
  assert(index <= debug_index());
  return read_(index);
}

template <typename T>
inline void VarTable<T>::write_control_var(size_t index, T val) {
  assert(index >= there_are_updates_index());
  assert(index <= debug_index());
  write_(index, val);
}

template <typename T>
inline void VarTable<T>::read_var(const Identifier* id) const {
  const auto itr = vtable_.find(id);
  assert(itr != vtable_.end());

  auto idx = itr->second.begin;
  for (size_t i = 0; i < itr->second.elements; ++i) {
    for (size_t j = 0; j < itr->second.words_per_element; ++j) {
      const volatile auto word = read_(idx);
      Evaluate().assign_word<T>(id, i, j, word);
      ++idx;
    } 
  }
}

template <typename T>
inline void VarTable<T>::write_var(const Identifier* id, const Bits& val) {
  const auto itr = vtable_.find(id);
  assert(itr != vtable_.end());
  assert(itr->second.elements == 1);

  auto idx = itr->second.begin;
  for (size_t j = 0; j < itr->second.words_per_element; ++j) {
    const volatile auto word = val.read_word<T>(j);
    write_(idx, word);
    ++idx;
  }
}

template <typename T>
inline void VarTable<T>::write_var(const Identifier* id, const Vector<Bits>& val) {
  const auto itr = vtable_.find(id);
  assert(itr != vtable_.end());
  assert(val.size() == itr->second.elements);

  auto idx = itr->second.begin;
  for (size_t i = 0; i < itr->second.elements; ++i) {
    for (size_t j = 0; j < itr->second.words_per_element; ++j) {
      const volatile auto word = val[i].read_word<T>(j);
      write_(idx, word);
      ++idx;
    }
  }
}

template <typename T>
inline T VarTable<T>::read_expr(const Expression* e) const {
  const auto itr = etable_.find(e);
  assert(itr != etable_.end());
  assert(itr->second.elements == 1);
  assert(itr->words_per_element == 1);

  return read_(var_size() + itr->second.begin);
}

template <typename T>
inline void VarTable<T>::write_expr(const Expression* e, T val) {
  const auto itr = etable_.find(e);
  assert(itr != etable_.end());
  assert(itr->second.elements == 1);
  assert(itr->second.words_per_element == 1);

  write_(var_size() + itr->second.begin, val);
}

template <typename T>
inline constexpr size_t VarTable<T>::bits_per_word() const {
  return 8*sizeof(T);
}

} // namespace cascade::avmm

#endif
