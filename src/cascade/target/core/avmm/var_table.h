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

template <size_t V, typename A, typename T>
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
    typedef std::function<T(A)> Read;
    typedef std::function<void(A, T)> Write;

    // Iterator Typedefs:
    typedef typename std::unordered_map<const Identifier*, const Row>::const_iterator const_iterator;
        
    // Constructors:
    VarTable();

    // Configuration Interface:
    VarTable& set_read(Read read);
    VarTable& set_write(Write write);

    // Inserts an element into the table.
    void insert(const Identifier* id);
    // Returns the number of words in the var table.
    size_t size() const;

    // Returns a pointer to an element in the table or end on failure
    const_iterator find(const Identifier* id) const;
    // Returns a pointer to the beginning of the table
    const_iterator begin() const;
    // Returns a pointer ot the end of the table
    const_iterator end() const;

    // Returns the starting index of this identifier.
    size_t index(const Identifier* id) const;
    // Returns the address of the there_are_updates control variable.
    size_t there_are_updates_index() const;
    // Returns the address of the apply_update control variable.
    size_t apply_update_index() const;
    // Returns the address of the task control variable.
    size_t there_were_tasks_index() const;
    // Returns the address of the resume control variable.
    size_t resume_index() const;
    // Returns the address of the reset control variable.
    size_t reset_index() const;
    // Returns the address of the open_loop control variable.
    size_t open_loop_index() const;
    // Returns the address of the feof control variable.
    size_t feof_index() const;
    // Reserved for debugging
    size_t debug_index() const;

    // Reads the value of a control variable
    T read_control_var(size_t index) const;
    // Writes the value of a control variable
    void write_control_var(size_t index, T val);

    // Reads the value of a variable
    void read_var(size_t slot, const Identifier* id) const; 
    // Writes the value of a scalar variable
    void write_var(size_t slot, const Identifier* id, const Bits& val);
    // Writes the value of an array variable
    void write_var(size_t slot, const Identifier* id, const Vector<Bits>& val);

  private:
    Read read_;
    Write write_;

    size_t next_index_;
    std::unordered_map<const Identifier*, const Row> vtable_;
};

template <size_t V, typename A, typename T>
inline VarTable<V,A,T>::VarTable() {
  next_index_ = 0;
}

template <size_t V, typename A, typename T>
inline VarTable<V,A,T>& VarTable<V,A,T>::set_read(Read read) {
  read_ = read;
  return *this;
}

template <size_t V, typename A, typename T>
inline VarTable<V,A,T>& VarTable<V,A,T>::set_write(Write write) {
  write_ = write;
  return *this;
}

template <size_t V, typename A, typename T>
inline void VarTable<V,A,T>::insert(const Identifier* id) {
  assert(find(id) == end());

  Row row;
  row.begin = next_index_;
  row.elements = 1;
  for (auto d : Evaluate().get_arity(id)) {
    row.elements *= d;
  }
  const auto* r = Resolve().get_resolution(id);
  assert(r != nullptr);
  row.bits_per_element = std::max(Evaluate().get_width(r), Evaluate().get_width(id));
  row.words_per_element = (row.bits_per_element + std::numeric_limits<T>::digits - 1) / std::numeric_limits<T>::digits;

  vtable_.insert(std::make_pair(id, row));
  next_index_ += (row.elements * row.words_per_element);
}

template <size_t V, typename A, typename T>
inline size_t VarTable<V,A,T>::size() const {
  return debug_index() + 1;
}

template <size_t V, typename A, typename T>
inline typename VarTable<V,A,T>::const_iterator VarTable<V,A,T>::find(const Identifier* id) const {
  return vtable_.find(id);
}

template <size_t V, typename A, typename T>
inline typename VarTable<V,A,T>::const_iterator VarTable<V,A,T>::begin() const {
  return vtable_.begin();
}

template <size_t V, typename A, typename T>
inline typename VarTable<V,A,T>::const_iterator VarTable<V,A,T>::end() const {
  return vtable_.end();
}

template <size_t V, typename A, typename T>
inline size_t VarTable<V,A,T>::index(const Identifier* id) const {
  const auto itr = vtable_.find(id);
  assert(itr != vtable_.end());
  return itr->second.index_;
}

template <size_t V, typename A, typename T>
inline size_t VarTable<V,A,T>::there_are_updates_index() const {
  return next_index_;
}

template <size_t V, typename A, typename T>
inline size_t VarTable<V,A,T>::apply_update_index() const {
  return next_index_ + 1;
}

template <size_t V, typename A, typename T>
inline size_t VarTable<V,A,T>::there_were_tasks_index() const {
  return next_index_ + 2;
}

template <size_t V, typename A, typename T>
inline size_t VarTable<V,A,T>::resume_index() const {
  return next_index_ + 3;
}

template <size_t V, typename A, typename T>
inline size_t VarTable<V,A,T>::reset_index() const {
  return next_index_ + 4;
}

template <size_t V, typename A, typename T>
inline size_t VarTable<V,A,T>::open_loop_index() const {
  return next_index_ + 5;
}

template <size_t V, typename A, typename T>
inline size_t VarTable<V,A,T>::feof_index() const {
  return next_index_ + 6;
}

template <size_t V, typename A, typename T>
inline size_t VarTable<V,A,T>::debug_index() const {
  return next_index_ + 7;
}

template <size_t V, typename A, typename T>
inline T VarTable<V,A,T>::read_control_var(size_t index) const {
  assert(index >= there_are_updates_index());
  assert(index <= debug_index());
  return read_(index);
}

template <size_t V, typename A, typename T>
inline void VarTable<V,A,T>::write_control_var(size_t index, T val) {
  assert(index >= there_are_updates_index());
  assert(index <= debug_index());
  write_(index, val);
}

template <size_t V, typename A, typename T>
inline void VarTable<V,A,T>::read_var(size_t slot, const Identifier* id) const {
  const auto itr = vtable_.find(id);
  assert(itr != vtable_.end());

  auto idx = itr->second.begin;
  for (size_t i = 0; i < itr->second.elements; ++i) {
    for (size_t j = 0; j < itr->second.words_per_element; ++j) {
      const volatile auto word = read_((slot << V) | idx);
      Evaluate().assign_word<T>(id, i, j, word);
      ++idx;
    } 
  }
}

template <size_t V, typename A, typename T>
inline void VarTable<V,A,T>::write_var(size_t slot, const Identifier* id, const Bits& val) {
  const auto itr = vtable_.find(id);
  assert(itr != vtable_.end());
  assert(itr->second.elements == 1);

  auto idx = itr->second.begin;
  for (size_t j = 0; j < itr->second.words_per_element; ++j) {
    const volatile auto word = val.read_word<T>(j);
    write_((slot << V) | idx, word);
    ++idx;
  }
}

template <size_t V, typename A, typename T>
inline void VarTable<V,A,T>::write_var(size_t slot, const Identifier* id, const Vector<Bits>& val) {
  const auto itr = vtable_.find(id);
  assert(itr != vtable_.end());
  assert(val.size() == itr->second.elements);

  auto idx = itr->second.begin;
  for (size_t i = 0; i < itr->second.elements; ++i) {
    for (size_t j = 0; j < itr->second.words_per_element; ++j) {
      const volatile auto word = val[i].read_word<T>(j);
      write_((slot << V) | idx, word);
      ++idx;
    }
  }
}

} // namespace cascade::avmm

#endif
