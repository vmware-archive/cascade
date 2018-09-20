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

#ifndef CASCADE_SRC_BASE_BITS_BITS_H
#define CASCADE_SRC_BASE_BITS_BITS_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdint.h>
#include <vector>
#include "src/base/serial/serializable.h"

namespace cascade {

// This class is the fundamental representation of a bit string. 

template <typename T>
class BitsBase : public Serializable {
  public:
    // Constructors:
    BitsBase();
    explicit BitsBase(bool b);
    BitsBase(size_t n, T val);
    BitsBase(const BitsBase& rhs) = default;
    BitsBase(BitsBase&& rhs) = default;
    BitsBase& operator=(const BitsBase& rhs) = default;
    BitsBase& operator=(BitsBase&& rhs) = default;
    ~BitsBase() override = default;
    
    // Serial I/O
    void read(std::istream& is, size_t base);
    void write(std::ostream& os, size_t base) const;
    size_t deserialize(std::istream& is) override;
    size_t serialize(std::ostream& os) const override;

    // Block I/O
    template <typename B>
    B read_word(size_t n);
    template <typename B>
    void write_word(size_t n, B b);

    // Casts:
    bool to_bool() const;
    T to_int() const;

    // Size:
    size_t size() const;
    void resize(size_t n);

    // Bitwise Operators:
    BitsBase& bitwise_and(const BitsBase& rhs);
    BitsBase& bitwise_or(const BitsBase& rhs);
    BitsBase& bitwise_xor(const BitsBase& rhs);
    BitsBase& bitwise_xnor(const BitsBase& rhs);
    BitsBase& bitwise_sll(const BitsBase& rhs);
    BitsBase& bitwise_sal(const BitsBase& rhs);
    BitsBase& bitwise_slr(const BitsBase& rhs);
    BitsBase& bitwise_sar(const BitsBase& rhs);
    BitsBase& bitwise_not();

    // Arithmetic Operators:
    BitsBase& arithmetic_plus();
    BitsBase& arithmetic_plus(const BitsBase& rhs);
    BitsBase& arithmetic_minus();
    BitsBase& arithmetic_minus(const BitsBase& rhs);
    BitsBase& arithmetic_multiply(const BitsBase& rhs);
    BitsBase& arithmetic_divide(const BitsBase& rhs);
    BitsBase& arithmetic_mod(const BitsBase& rhs);
    BitsBase& arithmetic_pow(const BitsBase& rhs);

    // Logical Operators:
    BitsBase& logical_and(const BitsBase& rhs);
    BitsBase& logical_or(const BitsBase& rhs);
    BitsBase& logical_not();
    BitsBase& logical_eq(const BitsBase& rhs);
    BitsBase& logical_ne(const BitsBase& rhs);
    BitsBase& logical_lt(const BitsBase& rhs);
    BitsBase& logical_lte(const BitsBase& rhs);
    BitsBase& logical_gt(const BitsBase& rhs);
    BitsBase& logical_gte(const BitsBase& rhs);

    // Reduction Operators:
    BitsBase& reduce_and();
    BitsBase& reduce_nand();
    BitsBase& reduce_or();
    BitsBase& reduce_nor();
    BitsBase& reduce_xor();
    BitsBase& reduce_xnor();

    // Concatenation Operators:
    BitsBase& concat(const BitsBase& rhs);

    // Sign manipulation
    BitsBase& to_signed();
    BitsBase& to_unsigned();

    // Slicing Operators:
    BitsBase& slice(size_t idx);
    BitsBase& slice(size_t msb, size_t lsb);

    // Slice Comparison Operators:
    bool eq(const BitsBase& rhs, size_t idx);
    bool eq(const BitsBase& rhs, size_t msb, size_t lsb);

    // Bitwise Operators:
    bool get(size_t idx) const;
    BitsBase& set(size_t idx, bool b);
    BitsBase& flip(size_t idx);

    // Assignment Operators:
    BitsBase& assign(const BitsBase& rhs);
    BitsBase& assign(size_t idx, const BitsBase& rhs);
    BitsBase& assign(size_t msb, size_t lsb, const BitsBase& rhs);

    // Built-in Operators:
    bool operator==(const BitsBase& rhs) const;
    bool operator!=(const BitsBase& rhs) const;
    bool operator<(const BitsBase& rhs) const;

  private:
    // Bit-string representation
    std::vector<T> val_;
    // Total number of bits in this string
    uint32_t size_;
    // How is this value being interpreted
    bool signed_;

    // Shift helpers 
    BitsBase& bitwise_sll_const(size_t samt);
    BitsBase& bitwise_sxr_const(size_t samt, bool arith);

    // Trims additional bits down to size_
    void trim();
    // Extends representation to n bits and zero pads
    void extend_to(size_t n); 
    // Shrinks size and calls trim
    void shrink_to(size_t n);
    // Shrinks to size one and sets val
    void shrink_to_bool(bool b);

    // Returns the number of bits in a word
    constexpr size_t bits_per_word() const;
    // Returns the number of bytes in a word
    constexpr size_t bytes_per_word() const;
};

typedef BitsBase<uint8_t>  Bits8;
typedef BitsBase<uint16_t> Bits16;
typedef BitsBase<uint32_t> Bits32;
typedef BitsBase<uint64_t> Bits64;

template <typename T>
inline BitsBase<T>::BitsBase() {
  val_.push_back(0);
  size_ = 1;
  signed_ = false;
}

template <typename T>
inline BitsBase<T>::BitsBase(bool b) {
  val_.push_back(b ? 1 : 0);
  size_ = 1;
  signed_ = false;
}

template <typename T>
inline BitsBase<T>::BitsBase(size_t n, T val) { 
  assert(n > 0);

  val_.push_back(val);
  size_ = n;
  signed_ = false;

  extend_to(size_);
  trim();
}

template <typename T>
inline void BitsBase<T>::read(std::istream& is, size_t base) {
  // TODO!!!!!!
}

template <typename T>
inline void BitsBase<T>::write(std::ostream& os, size_t base) const {
  // Binary, octal, and hex are easy
  if ((base == 2) || (base == 8) || (base == 16)) {
    // How many bits do we consume per character? Make a mask.
    const auto step = (base == 2) ? 1 : (base == 8) ? 3 : 4;
    const auto mask = (1 << step) - 1;
    // How far from the back of each word will we start extracting characters?
    const auto offset = (base == 8) ? (bits_per_word() % 3) : step;

    // Walk over the string with special handling for trailing zeros
    bool zeros = true;
    for (int i = val_.size()-1; i >= 0; --i) {
      for (int j = bits_per_word() - offset; j >= 0; j -= step) {
        const auto val = (val_[i] >> j) & mask;
        zeros = zeros && (val == 0);
        if (zeros) {
          continue;
        } else if (val > 9) {
          os << (char)('a' + val - 10);
        } else {
          os << val;
        }
      }
    }
    if (zeros) {
      os << "0";
    }
    return;
  }

  // No additional support unless base is 10
  if (base != 10) {
    assert(false);
    return;
  }
  
  // TODO!!!!!!!
}

template <typename T>
inline size_t BitsBase<T>::deserialize(std::istream& is) {
  uint32_t header;
  is.read((char*)&header, 4);

  extend_to(header & 0x7fffffff);
  signed_ = header & 0x80000000;

  for (auto& v : val_) {
    for (size_t i = 0; i < bytes_per_word(); ++i) {
      uint8_t b;
      is.read((char*)&b, 1);
      v |= (b << 8*i);
    }
  }

  return 4 + val_.size() * bytes_per_word();
}

template <typename T>
inline size_t BitsBase<T>::serialize(std::ostream& os) const {
  const uint32_t header = size_ | (signed_ ? 0x8000000 : 0);
  os.write((char*)&header, 4);

  for (auto v : val_) {
    for (size_t i = 0; i < bytes_per_word(); ++i) {
      os.write((char*)&v, 1);
      v >>= 8;
    }
  }

  return 4 + val_.size() * bytes_per_word();
}

template <typename T>
template <typename B>
inline B BitsBase<T>::read_word(size_t n) {
  // TODO!!!!!!
  return 0;
}

template <typename T>
template <typename B>
inline void BitsBase<T>::write_word(size_t n, B b) {
  // TODO!!!!!!
}

template <typename T>
inline bool BitsBase<T>::to_bool() const {
  for (const auto& v : val_) {
    if (v) {
      return true;
    }
  }
  return false;
}

template <typename T>
inline T BitsBase<T>::to_int() const {
  assert(size_ <= bits_per_word());
  return val_[0];
}

template <typename T>
inline size_t BitsBase<T>::size() const {
  return size_;
}

template <typename T>
inline void BitsBase<T>::resize(size_t n) {
  if (n < size_) {
    shrink_to(n);
  } else {
    extend_to(n);
  }
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_and(const BitsBase& rhs) {
  if (rhs.size_ > size_) {
    extend_to(rhs.size_);
  }
  for (size_t i = 0, ie = std::min(val_.size(), rhs.val_.size()); i < ie; ++i) {
    val_[i] &= rhs.val_[i];
  }
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_or(const BitsBase& rhs) {
  if (rhs.size_ > size_) {
    extend_to(rhs.size_);
  }
  for (size_t i = 0, ie = std::min(val_.size(), rhs.val_.size()); i < ie; ++i) {
    val_[i] |= rhs.val_[i];
  }
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_xor(const BitsBase& rhs) {
  if (rhs.size_ > size_) {
    extend_to(rhs.size_);
  }
  for (size_t i = 0, ie = std::min(val_.size(), rhs.val_.size()); i < ie; ++i) {
    val_[i] ^= rhs.val_[i];
  }
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_xnor(const BitsBase& rhs) {
  if (rhs.size_ > size_) {
    extend_to(rhs.size_);
  }
  for (size_t i = 0, ie = std::min(val_.size(), rhs.val_.size()); i < ie; ++i) {
    val_[i] ^= rhs.val_[i];
    val_[i] = ~val_[i];
  }
  // Careful: We can introduce trailing 1s here
  trim();
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_sll(const BitsBase& rhs) {
  const auto samt = rhs.to_int();
  if (samt > 0) {
    bitwise_sll_const(samt);
  }
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_sal(const BitsBase& rhs) {
  // Equivalent to sll
  return bitwise_sll(rhs);
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_slr(const BitsBase& rhs) {
  const auto samt = rhs.to_int();
  if (samt > 0) {
    bitwise_sxr_const(samt, false);
  }
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_sar(const BitsBase& rhs) {
  const auto samt = rhs.to_int();
  if (samt > 0) {
    bitwise_sxr_const(samt, true);
  }
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_not() {
  for (auto& v : val_) {
    v = ~v;
  }
  // Careful: We can introduce trailing 1s here
  trim();
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_plus() {
  // Does nothing.
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_plus(const BitsBase& rhs) {
  // TODO!!!!!!
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_minus() {
  // TODO!!!!!!
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_minus(const BitsBase& rhs) {
  // TODO!!!!!!
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_multiply(const BitsBase& rhs) {
  // TODO!!!!!!
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_divide(const BitsBase& rhs) {
  // TODO!!!!!!
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_mod(const BitsBase& rhs) {
  // TODO!!!!!!
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_pow(const BitsBase& rhs) {
  // TODO!!!!!!
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_and(const BitsBase& rhs) {
  shrink_to_bool(to_bool() && rhs.to_bool());
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_or(const BitsBase& rhs) {
  shrink_to_bool(to_bool() || rhs.to_bool());
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_not() {
  shrink_to_bool(!to_bool());
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_eq(const BitsBase& rhs) {
  for (size_t i = 0, ie = std::min(val_.size(), rhs.val_.size()); i < ie; ++i) {
    if (val_[i] != rhs.val_[i]) {
      shrink_to_bool(false);
      return *this;
    }
  }
  if (val_.size() < rhs.val_.size()) {
    for (size_t i = val_.size(), ie = rhs.val_.size(); i < ie; ++i) {
      if (rhs.val_[i]) {
        shrink_to_bool(false);
        return *this;
      }
    }
  } else if (val_.size() > rhs.val_.size()) {
    for (size_t i = rhs.val_.size(), ie = val_.size(); i < ie; ++i) {
      if (val_[i]) {
        shrink_to_bool(false);
        return *this;
      }
    }
  } 

  shrink_to_bool(true);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_ne(const BitsBase& rhs) {
  logical_eq(rhs);
  val_[0] = val_[0] ? 0 : 1;
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_lt(const BitsBase& rhs) {
  if (val_.size() < rhs.val_.size()) {
    for (int i = rhs.val_.size(), ie = val_.size(); i >= ie; --i) {
      if (rhs.val_[i]) {
        shrink_to_bool(true);
        return *this;
      }
    }
  } else if (val_.size() > rhs.val_.size()) {
    for (int i = val_.size(), ie = rhs.val_.size(); i >= ie; --i) {
      if (val_[i]) {
        shrink_to_bool(true);
        return *this;
      }
    }
  } 
  for (int i = std::min(val_.size(), rhs.val_.size()); i >= 0; --i) {
    if (val_[i] < rhs.val_[i]) {
      shrink_to_bool(true);
      return *this;
    }
  }

  shrink_to_bool(false);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_lte(const BitsBase& rhs) {
  if (val_.size() < rhs.val_.size()) {
    for (int i = rhs.val_.size(), ie = val_.size(); i >= ie; --i) {
      if (rhs.val_[i]) {
        shrink_to_bool(true);
        return *this;
      }
    }
  } else if (val_.size() > rhs.val_.size()) {
    for (int i = val_.size(), ie = rhs.val_.size(); i >= ie; --i) {
      if (val_[i]) {
        shrink_to_bool(true);
        return *this;
      }
    }
  } 
  for (int i = std::min(val_.size(), rhs.val_.size()); i >= 0; --i) {
    if (val_[i] <= rhs.val_[i]) {
      shrink_to_bool(true);
      return *this;
    }
  }

  shrink_to_bool(false);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_gt(const BitsBase& rhs) {
  logical_lte(rhs);
  val_[0] = val_[0] ? 0 : 1;
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_gte(const BitsBase& rhs) {
  logical_lt(rhs);
  val_[0] = val_[0] ? 0 : 1;
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::reduce_and() {
  for (size_t i = 0, ie = val_.size()-1; i < ie; ++i) {
    if (val_[0] != -1) {
      shrink_to_bool(false);
      return *this;
    }
  }
  const auto mask = (1 << (size_ % bits_per_word())) - 1;
  if ((val_.back() & mask) != mask) {
    shrink_to_bool(false);
    return *this;
  }
  shrink_to_bool(true);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::reduce_nand() {
  reduce_and();
  val_[0] = val_[0] ? 0 : 1;
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::reduce_or() {
  for (const auto& val : val_) {
    if (val_) {
      shrink_to_bool(true);
      return *this;
    }
  }
  shrink_to_bool(true);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::reduce_nor() {
  reduce_or();
  val_[0] = val_[0] ? 0 : 1;
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::reduce_xor() {
  size_t cnt = 0;
  for (const auto& v : val_) {
    cnt += __builtin_popcount(v);
  }
  shrink_to_bool(cnt % 2);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::reduce_xnor() {
  reduce_xor();
  val_[0] = val_[0] ? 0 : 1;
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::concat(const BitsBase& rhs) {
  extend_to(size_ + rhs.size_);
  bitwise_sll_const(rhs.size_);
  bitwise_or(rhs);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::to_signed() {
  signed_ = true;
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::to_unsigned() {
  signed_ = false;
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::slice(size_t idx) {
  // TODO!!!!!!
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::slice(size_t msb, size_t lsb) {
  // TODO!!!!!!
  return *this;
}

template <typename T>
inline bool BitsBase<T>::eq(const BitsBase& rhs, size_t idx) {
  // TODO...
  return false;
}

template <typename T>
inline bool BitsBase<T>::eq(const BitsBase& rhs, size_t msb, size_t lsb) {
  // TODO...
  return false;
}

template <typename T>
inline bool BitsBase<T>::get(size_t idx) const {
  assert(idx < size_);
  const auto widx = idx / bits_per_word();
  const auto bidx = idx % bits_per_word();
  return val_[widx] & (1 << bidx);
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::set(size_t idx, bool b) {
  assert(idx < size_);
  const auto widx = idx / bits_per_word();
  const auto bidx = idx % bits_per_word();

  if (b) {
    val_[widx] |= (1 << bidx);
  } else {
    val_[widx] &= ~(1 << bidx);
  }
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::flip(size_t idx) {
  assert(idx < size_);
  const auto widx = idx / bits_per_word();
  const auto bidx = idx % bits_per_word();
  const auto b = (val_[widx] >> bidx) & 1;

  if (!b) {
    val_[widx] |= (1 << bidx);
  } else {
    val_[widx] &= ~(1 << bidx);
  }

  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::assign(const BitsBase& rhs) {
  for (size_t i = 0, ie = std::min(val_.size(), rhs.val_.size()); i < ie; ++i) {
    val_[i] = rhs.val_[i];
  }
  for (size_t i = rhs.val_.size(), ie = val_.size(); i < ie; ++i) {
    val_[i] = 0;
  }
  trim();
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::assign(size_t idx, const BitsBase& rhs) {
  set(idx, rhs.val_[0] & 1);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::assign(size_t msb, size_t lsb, const BitsBase& rhs) {
  // Corner case: Is this range one bit?
  if (msb == lsb) {
    assign(msb, rhs);
  }

  // TODO!!!!!

  return *this;
}

template <typename T>
inline bool BitsBase<T>::operator==(const BitsBase& rhs) const {
  return size_ == rhs.size_ && logical_eq(rhs);
}

template <typename T>
inline bool BitsBase<T>::operator!=(const BitsBase& rhs) const {
  return !(*this == rhs);
}

template <typename T>
inline bool BitsBase<T>::operator<(const BitsBase& rhs) const {
  return size_ < rhs.size_ || logical_lt(rhs);
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_sll_const(size_t samt) {
  // This algorithm works from highest to lowest order, one word at a time
  // word: The current word we're looking at
  // top/bottom: The words that will be shifted into word; word can equal top

  // How many words ahead is bottom?
  const auto delta = (samt + bits_per_word() - 1) / bits_per_word();
  // How many bits are we taking from bottom and shifting top?
  const auto bamt = samt % bits_per_word();
  // Create a mask for extracting the highest bamt bits from bottom
  const auto mamt = bits_per_word() - bamt;
  const auto mask = ((1 << bamt) - 1) << mamt;

  // Work our way down until bottom hits zero
  int w = val_.size() - 1;
  for (int b = w-delta; b >= 0; --w, --b) {
    if (bamt == 0) {
      val_[w] = val_[b];
    } else {
      val_[w] = (val_[b+1] << bamt) | ((val_[b] & mask) >> mamt);
    }
  }
  // There's one more block to build where bottom is implicitly zero
  val_[w--] = (bamt == 0) ? 0 : (val_[0] << bamt);
  // Everything else is zero
  for (; w >= 0; --w) {
    val_[w] = 0;
  }

  // Trim the top and we're done
  trim();
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_sxr_const(size_t samt, bool arith) {
  // This algorithm works from lowest to highest order, one word at a time
  // word: The current word we're looking at
  // top/bottom: The words that will be shifted into word; word can equal bottom

  // Is the highest order bit a 1 and do we care?
  const auto idx = (size_-1) % bits_per_word();
  const auto hob = arith && (val_.back() & (1 << idx)); 
  // How many words ahead is top?
  const auto delta = (samt + bits_per_word() - 1) / bits_per_word();
  // How many bits are we taking from top and shifting bottom?
  const auto bamt = samt % bits_per_word();
  // Create a mask for extracting the lowest bamt bits from top
  const auto mamt = bits_per_word() - bamt;
  const auto mask = (1 << bamt) - 1;

  // Work our way up until top goes out of range
  int w = 0;
  for (int t = w+delta, te = val_.size(); t < te; ++w, ++t) {
    if (bamt == 0) {
      val_[w] = val_[t];
    } else {
      val_[w] = (val_[t-1] >> bamt) | ((val_[t] & mask) << mamt);
    }
  }
  // There's one more block to build where top is implicitly zero
  if (hob) {
    val_[w++] = (bamt == 0) ? -1 : ((val_.back() >> bamt) | (mask << mamt));
  } else {
    val_[w++] = (bamt == 0) ? 0 : (val_.back() >> bamt);
  }
  // Everything else is zero or padded 1s
  for (size_t we = val_.size(); w < we; ++w) {
    val_[w] = hob ? -1 : 0;
  }

  // Trim since we could have introduced trailing 1s
  trim();
  return *this;
}

template <typename T>
void BitsBase<T>::trim() {
  // How many bits do we care about in the top word?
  const auto trailing = size_ % bits_per_word();
  // Zero means we're full
  if (trailing == 0) {
    return;
  }
  // Otherwise, mask these off
  const auto mask = (1 << trailing) - 1;
  val_.back() &= mask;
}
    
template <typename T>
void BitsBase<T>::extend_to(size_t n) {
  const auto words = (n + bits_per_word() - 1) / bits_per_word();
  val_.resize(words);
  size_ = n;
}
   
template <typename T>
void BitsBase<T>::shrink_to(size_t n) {
  const auto words = (n + bits_per_word() - 1) / bits_per_word();
  val_.resize(words);
  size_ = n;
  trim();
}

template <typename T>
void BitsBase<T>::shrink_to_bool(bool b) {
  val_.resize(1);
  val_[0] = b ? 1 : 0;
  size_ = 1;
}

template <typename T>
constexpr size_t BitsBase<T>::bits_per_word() const {
  return 8 * bytes_per_word();
}

template <typename T>
constexpr size_t BitsBase<T>::bytes_per_word() const {
  return sizeof(T);
}

} // namespace cascade

#endif
