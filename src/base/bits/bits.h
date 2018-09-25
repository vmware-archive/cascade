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
#include <cctype>
#include <iostream>
#include <stdint.h>
#include <type_traits>
#include <vector>
#include "src/base/serial/serializable.h"

namespace cascade {

// This class is the fundamental representation of a bit string. 

template <typename T>
class BitsBase : public Serializable {
  private:
    typedef 
      typename std::conditional<sizeof(T) == 1, uint16_t,
      typename std::conditional<sizeof(T) == 2, uint32_t,
      typename std::conditional<sizeof(T) == 4, uint64_t, __uint128_t>::type>::type>::type BigT;

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
    bool operator<=(const BitsBase& rhs) const;
    bool operator>(const BitsBase& rhs) const;
    bool operator>=(const BitsBase& rhs) const;

  private:
    // Bit-string representation
    std::vector<T> val_;
    // Total number of bits in this string
    uint32_t size_;
    // How is this value being interpreted
    bool signed_;

    // I/O Helpers
    // 
    // Reads a number in base 2, 8, or 16, updates size as necessary
    void read_2_8_16(std::istream& is, size_t base);
    // Reads a number in base 10, updates size as necessary
    void read_10(std::istream& is);
    // Writes a number in base 2, 8, or 16
    void write_2_8_16(std::ostream& os, size_t base) const;
    // Writes a number in base 10
    void write_10(std::ostream& os) const;
    // Halves a decimal value, stored with msb in index 0
    void dec_halve(std::vector<uint8_t>& s) const;
    // Returns true if a decimal value, stored with msb in index 0 is 0
    bool dec_zero(const std::vector<uint8_t>& s) const;
    // Doubles a decimal value, stored with lsb in index 0
    void dec_double(std::vector<uint8_t>& s) const;
    // Increments a decimal value, stored with lsb in index 0
    void dec_inc(std::vector<uint8_t>& s) const;

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

using Bits8 = BitsBase<uint8_t>;
using Bits16 = BitsBase<uint16_t>;
using Bits32 = BitsBase<uint32_t>;
using Bits64 = BitsBase<uint64_t>;
using Bits = Bits64;

template <typename T>
inline BitsBase<T>::BitsBase() {
  shrink_to_bool(false);
  signed_ = false;
}

template <typename T>
inline BitsBase<T>::BitsBase(bool b) {
  shrink_to_bool(b);
  signed_ = false;
}

template <typename T>
inline BitsBase<T>::BitsBase(size_t n, T val) { 
  assert(n > 0);

  val_.push_back(val);
  extend_to(n);
  trim();
  signed_ = false;
}

template <typename T>
inline void BitsBase<T>::read(std::istream& is, size_t base) {
  switch (base) {
    case 2:
    case 8:
    case 16:
      return read_2_8_16(is, base);
    case 10:
      return read_10(is);
    default:
      assert(false);
  }
}

template <typename T>
inline void BitsBase<T>::write(std::ostream& os, size_t base) const {
  switch (base) {
    case 2:
    case 8:
    case 16:
      return write_2_8_16(os, base);
    case 10:
      return write_10(os);
    default:
      assert(false);
  }
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
      v |= (T(b) << 8*i);
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
  (void) n;
  // TODO!!!!!!
  return 0;
}

template <typename T>
template <typename B>
inline void BitsBase<T>::write_word(size_t n, B b) {
  (void) n;
  (void) b;
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
  } else if (n > size_) {
    extend_to(n);
  }
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_and(const BitsBase& rhs) {
  if (rhs.size_ > size_) {
    extend_to(rhs.size_);
  }
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] &= (i < rhs.val_.size() ? rhs.val_[i] : T(0));
  }
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_or(const BitsBase& rhs) {
  if (rhs.size_ > size_) {
    extend_to(rhs.size_);
  }
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] |= (i < rhs.val_.size() ? rhs.val_[i] : T(0));
  }
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_xor(const BitsBase& rhs) {
  if (rhs.size_ > size_) {
    extend_to(rhs.size_);
  }
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] ^= (i < rhs.val_.size() ? rhs.val_[i] : T(0));
  }
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_xnor(const BitsBase& rhs) {
  if (rhs.size_ > size_) {
    extend_to(rhs.size_);
  }
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] ^= (i < rhs.val_.size() ? rhs.val_[i] : T(0));
    val_[i] = ~(i < rhs.val_.size() ? val_[i] : T(0));
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
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = ~val_[i];
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
  if (rhs.size_ > size_) {
    extend_to(rhs.size_);
  }

  T carry = 0;
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    const T sum = val_[i] + (i < rhs.val_.size() ? rhs.val_[i] : T(0)) + carry;
    if (carry) {
      carry = (sum <= val_[i]) ? 1 : 0; 
    } else {
      carry = (sum < val_[i]) ? 1 : 0;
    }
    val_[i] = sum;
  }

  trim();
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_minus() {
  bitwise_not();
  // TODO: Add one
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_minus(const BitsBase& rhs) {
  // TODO: What about unsigned subtraction?
  if (rhs.size_ > size_) {
    extend_to(rhs.size_);
  }

  T carry = 0;
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    const T dif = val_[i] - (i < rhs.val_.size() ? rhs.val_[i] : T(0)) - carry;
    if (carry) {
      carry = (dif >= val_[i]) ? 1 : 0; 
    } else {
      carry = (dif > val_[i]) ? 1 : 0;
    }
    val_[i] = dif;
  }

  trim();
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_multiply(const BitsBase& rhs) {
  // TODO: What about unsigned multiplication?
  if (rhs.size_ > size_) {
    extend_to(rhs.size_);
  }

  // This is the optimized space algorithm described in wiki's multiplication
  // algorithm article. The code is simplified here, as we can assume that both
  // inputs and the result are capped at N words. 

  const auto N = val_.size();
  std::vector<T> product(N, 0);
  BigT tot = 0;
  for (size_t ri = 0; ri < N; ++ri) {
    for (size_t bi = 0; bi <= ri; ++bi) {
      size_t ai = ri - bi;
      tot += (BigT(val_[ai]) * BigT(bi < rhs.val_.size() ? rhs.val_[bi] : 0));
    }
    product[ri] = (T)tot;
    tot >>= bits_per_word();
  }

  val_ = product;
  trim();
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_divide(const BitsBase& rhs) {
  if (rhs.size_ > size_) {
    extend_to(rhs.size_);
  }

  assert(val_.size() == 1);
  assert(rhs.val_.size() == 1);

  val_[0] /= rhs.val_[0];

  trim();
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_mod(const BitsBase& rhs) {
  if (rhs.size_ > size_) {
    extend_to(rhs.size_);
  }

  assert(val_.size() == 1);
  assert(rhs.val_.size() == 1);

  val_[0] %= rhs.val_[0];

  trim();
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::arithmetic_pow(const BitsBase& rhs) {
  (void) rhs;
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
  shrink_to_bool(*this == rhs);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_ne(const BitsBase& rhs) {
  shrink_to_bool(*this != rhs);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_lt(const BitsBase& rhs) {
  shrink_to_bool(*this < rhs);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_lte(const BitsBase& rhs) {
  shrink_to_bool(*this <= rhs);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_gt(const BitsBase& rhs) {
  shrink_to_bool(*this > rhs);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::logical_gte(const BitsBase& rhs) {
  shrink_to_bool(*this >= rhs);
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::reduce_and() {
  for (size_t i = 0, ie = val_.size()-1; i < ie; ++i) {
    if (val_[i] != T(-1)) {
      shrink_to_bool(false);
      return *this;
    }
  }
  const auto mask = (T(1) << (size_ % bits_per_word())) - 1;
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
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    if (val_[i]) {
      shrink_to_bool(true);
      return *this;
    }
  }
  shrink_to_bool(false);
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
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    cnt += __builtin_popcount(val_[i]);
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
  shrink_to_bool(get(idx));
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::slice(size_t msb, size_t lsb) {
  assert(msb < size_);
  assert(msb >= lsb);

  // Corner Case: Is this range 1 bit?
  if (msb == lsb) {
    return slice(msb);
  }
  if (lsb > 0) {
    bitwise_sxr_const(lsb, false);
  }
  shrink_to(msb-lsb+1);
  return *this;
}

template <typename T>
inline bool BitsBase<T>::eq(const BitsBase& rhs, size_t idx) {
  assert(idx < size_);
  return get(idx) == rhs.get(0);
}

template <typename T>
inline bool BitsBase<T>::eq(const BitsBase& rhs, size_t msb, size_t lsb) {
  assert(msb < size_);
  assert(msb >= lsb);

  // Corner Case: Is this a single bit range?
  if (msb == lsb) {
    return eq(rhs, msb);
  }

  // Compute the size of this slice 
  const auto slice = msb - lsb + 1;
  // How many full words does it span and what's left over at the end?
  const auto fspan = slice / bits_per_word();
  const auto lover = slice % bits_per_word();
  // Compute indices and offsets
  const auto lower = lsb / bits_per_word();
  const auto loff = lsb % bits_per_word();
  const auto uoff = bits_per_word() - loff;

  // Common Case: Full word comparison
  for (size_t i = 0; i < fspan; ++i) {
    auto word = (val_[lower+i] >> loff);
    if (loff > 0) {
      word |= (val_[lower+i+1] << uoff);
    } 
    if (word != rhs.val_[i]) {
      return false;
    } 
  }

  // Nothing to do if there are no leftovers:
  if (lover == 0) {
    return true;
  }

  // Edge Case: Compare the remaining bits
  auto word = (val_[lower+fspan] >> loff);
  if ((loff > 0) && ((lower+fspan+1) < val_.size())) {
    word |= (val_[lower+fspan+1] << uoff);
  } 
  const auto mask = (T(1) << lover) - 1;
  return (word & mask) == (rhs.val_[fspan] & mask);
}

template <typename T>
inline bool BitsBase<T>::get(size_t idx) const {
  assert(idx < size_);
  const auto widx = idx / bits_per_word();
  const auto bidx = idx % bits_per_word();
  return val_[widx] & (T(1) << bidx);
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::set(size_t idx, bool b) {
  assert(idx < size_);
  const auto widx = idx / bits_per_word();
  const auto bidx = idx % bits_per_word();

  if (b) {
    val_[widx] |= (T(1) << bidx);
  } else {
    val_[widx] &= ~(T(1) << bidx);
  }
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::flip(size_t idx) {
  assert(idx < size_);
  const auto widx = idx / bits_per_word();
  const auto bidx = idx % bits_per_word();
  const auto b = (val_[widx] >> bidx) & T(1);

  if (!b) {
    val_[widx] |= (T(1) << bidx);
  } else {
    val_[widx] &= ~(T(1) << bidx);
  }

  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::assign(const BitsBase& rhs) {
  // Copy words
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = i < rhs.val_.size() ? rhs.val_[i] : 0;
  }
  // Trim just in case the bits have different sizes
  trim();
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::assign(size_t idx, const BitsBase& rhs) {
  assert(idx < size_);
  set(idx, rhs.val_[0] & T(1));
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::assign(size_t msb, size_t lsb, const BitsBase& rhs) {
  assert(msb < size_);
  assert(msb >= lsb);

  // Corner case: Is this range one bit?
  if (msb == lsb) {
    assign(msb, rhs);
  }

  // Compute the size of this slice 
  const auto slice = msb - lsb + 1;
  // How many full words does it span and what's left over at the end?
  const auto fspan = slice / bits_per_word();
  const auto lover = slice % bits_per_word();
  // Compute indices and offsets
  const auto lower = lsb / bits_per_word();
  const auto loff = lsb % bits_per_word();
  const auto uoff = bits_per_word() - loff;
  const auto mask = (T(1) << loff) - 1;

  // Common Case: Copy entire words
  for (size_t i = 0; i < fspan; ++i) {
    val_[lower+i] &= mask;
    val_[lower+i] |= (rhs.val_[i] << loff);
    if (loff > 0) {
      val_[lower+i+1] &= ~mask;
      val_[lower+i+1] |= (rhs.val_[i] >> uoff);
    } 
  }

  // Nothing to do if there are no leftovers
  if (lover == 0) {
    return *this;
  }

  // Edge Case: Copy the remaining bits
  const auto lmask = (T(1) << lover) - 1;
  val_[lower+fspan] &= ~(lmask << loff);
  val_[lower+fspan] |= ((rhs.val_[fspan] & lmask) << loff);

  if ((lover + loff) > bits_per_word()) {
    const auto delta = lover + loff - bits_per_word();
    const auto hmask = (T(1) << delta) - 1;
    val_[lower+fspan+1] &= ~hmask;
    val_[lower+fspan+1] |= ((rhs.val_[fspan] >> (lover-delta)) & hmask);
  }

  return *this;
}

template <typename T>
inline bool BitsBase<T>::operator==(const BitsBase& rhs) const {
  for (size_t i = 0, ie = std::min(val_.size(), rhs.val_.size()); i < ie; ++i) {
    if (val_[i] != rhs.val_[i]) {
      return false;
    }
  }
  if (val_.size() < rhs.val_.size()) {
    for (size_t i = val_.size(), ie = rhs.val_.size(); i < ie; ++i) {
      if (rhs.val_[i]) {
        return false;
      }
    }
  } else if (val_.size() > rhs.val_.size()) {
    for (size_t i = rhs.val_.size(), ie = val_.size(); i < ie; ++i) {
      if (val_[i]) {
        return false;
      }
    }
  } 
  return true;
}

template <typename T>
inline bool BitsBase<T>::operator!=(const BitsBase& rhs) const {
  return !(*this == rhs);
}

template <typename T>
inline bool BitsBase<T>::operator<(const BitsBase& rhs) const {
  if (val_.size() < rhs.val_.size()) {
    for (size_t i = val_.size(), ie = rhs.val_.size(); i < ie; ++i) {
      if (rhs.val_[i]) {
        return true;
      }
    }
  } else if (val_.size() > rhs.val_.size()) {
    for (size_t i = rhs.val_.size(), ie = val_.size(); i < ie; ++i) {
      if (val_[i]) {
        return false;
      }
    }
  } 
  for (int i = std::min(val_.size(), rhs.val_.size())-1; i >= 0; --i) {
    if (val_[i] < rhs.val_[i]) {
      return true;
    } else if (val_[i] > rhs.val_[i]) {
      return false;
    }
  }
  return false;
}

template <typename T>
inline bool BitsBase<T>::operator<=(const BitsBase& rhs) const {
  if (val_.size() < rhs.val_.size()) {
    for (size_t i = val_.size(), ie = rhs.val_.size(); i < ie; ++i) {
      if (rhs.val_[i]) {
        return true;
      }
    }
  } else if (val_.size() > rhs.val_.size()) {
    for (size_t i = rhs.val_.size(), ie = val_.size(); i < ie; ++i) {
      if (val_[i]) {
        return false;
      }
    }
  } 
  for (int i = std::min(val_.size(), rhs.val_.size())-1; i >= 0; --i) {
    if (val_[i] < rhs.val_[i]) {
      return true;
    } else if (val_[i] > rhs.val_[i]) {
      return false;
    }
  }
  return true;
}

template <typename T>
inline bool BitsBase<T>::operator>(const BitsBase& rhs) const {
  return !(*this <= rhs);
}

template <typename T>
inline bool BitsBase<T>::operator>=(const BitsBase& rhs) const {
  return !(*this < rhs);
}

template <typename T>
inline void BitsBase<T>::read_2_8_16(std::istream& is, size_t base) {
  // Input Buffer:
  std::vector<uint8_t> buf;
  while (true) {
    auto c = is.get();
    if (isspace(c) || is.eof()) {
      break;
    } else if (isalpha(c)) {
      buf.push_back(c-'a'+10);
    } else {
      buf.push_back(c-'0');
    }
  }  

  // Reset internal state
  shrink_to_bool(false);
  // TODO... set signed to false?

  // How many bits do we generate per character?
  const auto step = (base == 2) ? 1 : (base == 8) ? 3 : 4;

  // Walk over the buffer from lowest to highest order (that's in reverse)
  size_t idx = 0;
  size_t total = 0;
  for (int i = buf.size()-1; i >= 0; --i) {
    // Append bits
    val_.back() |= (T(buf[i]) << idx);
    idx += step;
    total += step;
    extend_to(total);
    // Easy case: Step divides words evenly
    if (idx == bits_per_word()) {
      idx = 0;
      continue;
    }
    // Hard case: Bit overflow
    if (idx > bits_per_word()) {
      idx %= bits_per_word();
      val_.back() |= (buf[i] >> (step-idx));
      continue;
    }
  }

  // Trim trailing 0s (but leave at least one if this number is zero!)
  while ((size_ > 1) && !get(size_-1)) {
    shrink_to(size_-1);
  }
}

template <typename T>
inline void BitsBase<T>::read_10(std::istream& is) {
  // Input Buffer:
  std::vector<uint8_t> buf;
  while (true) {
    auto c = is.get();
    if (isspace(c) || is.eof()) {
      break;
    }
    buf.push_back(c-'0');
  }

  // Reset interal state
  shrink_to_bool(false);
  // TODO... set signed to false?

  // Halve decimal string until it becomes zero. Add 1s when least significant
  // digit is odd, 0s otherwise
  for (size_t i = 1; !dec_zero(buf); ++i) {
    extend_to(i);
    set(i-1, buf.back() % 2); 
    dec_halve(buf);
  }
}

template <typename T>
inline void BitsBase<T>::write_2_8_16(std::ostream& os, size_t base) const {
  // Output Buffer:
  std::vector<uint8_t> buf;

  // How many bits do we consume per character? Make a mask.
  const auto step = (base == 2) ? 1 : (base == 8) ? 3 : 4;
  const auto mask = (T(1) << step) - 1;

  // Walk over the string from lowest to highest order
  size_t idx = 0;
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    while (true) {
      // Extract mask bits 
      buf.push_back((val_[i] >> idx) & mask);
      idx += step;
      // Easy case: Step divides words evenly 
      if (idx == bits_per_word()) {
        idx = 0;
        break;
      } 
      // Hard case: Look ahead for more bits
      if (idx > bits_per_word()) {
        idx %= bits_per_word();
        if ((i+1) != ie) {
          buf.back() |= (val_[i+1] & ((T(1) << idx) - 1)) << (step - idx);
        }
        break;
      }
    }
  }

  // Print the result
  bool zero = true;
  for (int i = buf.size()-1; i >= 0; --i) {
    if (zero && (buf[i] == 0)) {
      continue;
    } else {
      zero = false;
      if (buf[i] > 9) {
        os << (char)('a' + buf[i] - 10);
      } else {
        os << (int)buf[i];
      }
    }
  }
  if (zero) {
    os << "0";  
  }
}


template <typename T>
inline void BitsBase<T>::write_10(std::ostream& os) const {
  // Output Buffer (lsb in index 0):
  std::vector<uint8_t> buf;
  buf.push_back(0);

  // Walk binary string from highest to lowest.
  // Double result each time, add 1s as they appear.
  for (int i = size_-1; i >= 0; --i) {
    dec_double(buf);
    if (get(i)) {
      dec_inc(buf);
    }
  }
  // Print the result
  for (int i = buf.size()-1; i >= 0; --i) {
    os << (int)buf[i];
  }
}

template <typename T>
inline void BitsBase<T>::dec_halve(std::vector<uint8_t>& s) const {
  auto next_carry = 0;
  for (size_t i = 0, ie = s.size(); i < ie; ++i) {
    auto carry = next_carry;
    next_carry = (s[i] % 2) ? 5 : 0;
    s[i] = s[i] / 2 + carry;
  }
}

template <typename T>
inline bool BitsBase<T>::dec_zero(const std::vector<uint8_t>& s) const {
  for (const auto& v : s) {
    if (v) {
      return false;
    }
  }
  return true;
}

template <typename T>
inline void BitsBase<T>::dec_double(std::vector<uint8_t>& s) const {
  auto carry = 0;
  for (size_t i = 0, ie = s.size(); i < ie; ++i) {
    s[i] = 2 * s[i] + carry;
    if (s[i] >= 10) {
      s[i] -= 10;
      carry = 1;
    } else {
      carry = 0;
    }
  }
  if (carry) {
    s.push_back(1);
  }
}

template <typename T>
inline void BitsBase<T>::dec_inc(std::vector<uint8_t>& s) const {
  for (size_t i = 0, ie = s.size(); i < ie; ++i) {
    if (++s[i] == 10) {
      s[i] = 0;
    } else {
      return;
    }
  }
  s.push_back(1);
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_sll_const(size_t samt) {
  assert(samt > 0);

  // This algorithm works from highest to lowest order, one word at a time
  // word: The current word we're looking at
  // top/bottom: The words that will be shifted into word; word can equal top

  // How many words ahead is bottom?
  const auto delta = (samt + bits_per_word() - 1) / bits_per_word();
  // How many bits are we taking from bottom and shifting top?
  const auto bamt = samt % bits_per_word();
  // Create a mask for extracting the highest bamt bits from bottom
  const auto mamt = bits_per_word() - bamt;
  const auto mask = ((T(1) << bamt) - 1) << mamt;

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
  val_[w--] = (bamt == 0) ? T(0) : (val_[0] << bamt);
  // Everything else is zero
  for (; w >= 0; --w) {
    val_[w] = T(0);
  }

  // Trim the top and we're done
  trim();
  return *this;
}

template <typename T>
inline BitsBase<T>& BitsBase<T>::bitwise_sxr_const(size_t samt, bool arith) {
  assert(samt > 0);

  // This algorithm works from lowest to highest order, one word at a time
  // word: The current word we're looking at
  // top/bottom: The words that will be shifted into word; word can equal bottom

  // Is the highest order bit a 1 and do we care?
  const auto idx = (size_-1) % bits_per_word();
  const auto hob = arith && (val_.back() & (T(1) << idx)); 
  // How many words ahead is top?
  const auto delta = (samt + bits_per_word() - 1) / bits_per_word();
  // How many bits are we taking from top and shifting bottom?
  const auto bamt = samt % bits_per_word();
  // Create a mask for extracting the lowest bamt bits from top
  const auto mamt = bits_per_word() - bamt;
  const auto mask = (T(1) << bamt) - 1;

  // Work our way up until top goes out of range
  size_t w = 0;
  for (size_t t = w+delta, te = val_.size(); t < te; ++w, ++t) {
    if (bamt == 0) {
      val_[w] = val_[t];
    } else {
      val_[w] = (val_[t-1] >> bamt) | ((val_[t] & mask) << mamt);
    }
  }
  // There's one more block to build where top is implicitly zero
  if (hob) {
    val_[w++] = (bamt == 0) ? T(-1) : ((val_.back() >> bamt) | (mask << mamt));
  } else {
    val_[w++] = (bamt == 0) ? T(0) : (val_.back() >> bamt);
  }
  // Everything else is zero or padded 1s
  for (size_t we = val_.size(); w < we; ++w) {
    val_[w] = hob ? T(-1) : T(0);
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
  const auto mask = (T(1) << trailing) - 1;
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
