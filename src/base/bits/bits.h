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
#include <cmath>
#include <iostream>
#include <stdint.h>
#include <string>
#include <type_traits>
#include <vector>
#include "src/base/container/vector.h"
#include "src/base/serial/serializable.h"

namespace cascade {

// This class is the fundamental representation of a bit string. 

template <typename T, typename BT, typename ST>
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
    B read_word(size_t n) const;
    template <typename B>
    void write_word(size_t n, B b);

    // Casts:
    bool to_bool() const;
    T to_int() const;

    // Size:
    size_t size() const;
    void resize(size_t n);

    // Type:
    bool is_signed() const;
    void set_signed(bool s);

    // Bitwise Operators: 
    //
    // Apply a bitwise operator to this and rhs and store the result in res.
    // These methods all assume equivalent bit-width between operands and
    // destination and so do not perform sign extension.
    void bitwise_and(const BitsBase& rhs, BitsBase& res) const;
    void bitwise_or(const BitsBase& rhs, BitsBase& res) const;
    void bitwise_xor(const BitsBase& rhs, BitsBase& res) const;
    void bitwise_xnor(const BitsBase& rhs, BitsBase& res) const;
    void bitwise_sll(const BitsBase& rhs, BitsBase& res) const;
    void bitwise_sal(const BitsBase& rhs, BitsBase& res) const;
    void bitwise_slr(const BitsBase& rhs, BitsBase& res) const;
    void bitwise_sar(const BitsBase& rhs, BitsBase& res) const;
    void bitwise_not(BitsBase& res) const;

    // Arithmetic Operators:
    //
    // Apply an arithmetic operator to this and rhs and store the result in
    // res.  These methods all assume equivalent bit-width between operands and
    // destination and so do not perform sign extension.
    void arithmetic_plus(BitsBase& res) const;
    void arithmetic_plus(const BitsBase& rhs, BitsBase& res) const;
    void arithmetic_minus(BitsBase& res) const;
    void arithmetic_minus(const BitsBase& rhs, BitsBase& res) const;
    void arithmetic_multiply(const BitsBase& rhs, BitsBase& res) const;
    void arithmetic_divide(const BitsBase& rhs, BitsBase& res) const;
    void arithmetic_mod(const BitsBase& rhs, BitsBase& res) const;
    void arithmetic_pow(const BitsBase& rhs, BitsBase& res) const;

    // Logical Operators:
    //
    // Apply a logical operator to this and rhs and store the result in res.
    // These methods all assume equivalent bit-width between operands and
    // single bit-width in their destination and so do not perform sign
    // extension.
    void logical_and(const BitsBase& rhs, BitsBase& res) const;
    void logical_or(const BitsBase& rhs, BitsBase& res) const;
    void logical_not(BitsBase& res) const;
    void logical_eq(const BitsBase& rhs, BitsBase& res) const;
    void logical_ne(const BitsBase& rhs, BitsBase& res) const;
    void logical_lt(const BitsBase& rhs, BitsBase& res) const;
    void logical_lte(const BitsBase& rhs, BitsBase& res) const;
    void logical_gt(const BitsBase& rhs, BitsBase& res) const;
    void logical_gte(const BitsBase& rhs, BitsBase& res) const;

    // Reduction Operators:
    //
    // Apply a reduction operator to this and store the result in res These
    // methods all assume single bit-width in their destination and so do not
    // perform sign extension.
    void reduce_and(BitsBase& res) const;
    void reduce_nand(BitsBase& res) const;
    void reduce_or(BitsBase& res) const;
    void reduce_nor(BitsBase& res) const;
    void reduce_xor(BitsBase& res) const;
    void reduce_xnor(BitsBase& res) const;

    // Comparison Operators:
    //
    // Check for equality. These methods make no assumptions made with respect
    // to sizes and handle sign-extension for rhs as necessary.
    bool eq(const BitsBase& rhs) const;
    bool eq(size_t idx, const BitsBase& rhs) const;
    bool eq(size_t msb, size_t lsb, const BitsBase& rhs) const;

    // Assignment Operators:
    //
    // Assign bits. These methods make no assumptions made with respect to
    // sizes and handle sign-extension for rhs as necessary.
    void assign(const BitsBase& rhs);
    void assign(size_t idx, const BitsBase& rhs);
    void assign(size_t msb, size_t lsb, const BitsBase& rhs);
    void assign(const BitsBase& rhs, size_t idx);
    void assign(const BitsBase& rhs, size_t msb, size_t lsb);

    // Concatenation Operations:
    void concat(const BitsBase& rhs);

    // Bitwise Operators:
    bool get(size_t idx) const;
    BitsBase& set(size_t idx, bool b);
    BitsBase& flip(size_t idx);

    // Built-in Operators:
    // Logical comparison, assumes equal operand sizes
    bool operator==(const BitsBase& rhs) const;
    bool operator!=(const BitsBase& rhs) const;
    bool operator<(const BitsBase& rhs) const;
    bool operator<=(const BitsBase& rhs) const;
    bool operator>(const BitsBase& rhs) const;
    bool operator>=(const BitsBase& rhs) const;

  private:
    // Bit-string representation
    Vector<T> val_;
    // Total number of bits in this string
    uint32_t size_;
    // How is this value being interpreted
    bool signed_;

    // Reads a number in base 2, 8, or 16, updates size as necessary
    void read_2_8_16(std::istream& is, size_t base);
    // Reads a number in base 10, updates size as necessary
    void read_10(std::istream& is);
    // Writes a number in base 2, 8, or 16
    void write_2_8_16(std::ostream& os, size_t base) const;
    // Writes a number in base 10
    void write_10(std::ostream& os) const;
    // Halves a decimal value, stored as a string
    void dec_halve(std::string& s) const;
    // Returns true if a decimal value, stored as a string is 0
    bool dec_zero(const std::string& s) const;
    // Doubles a decimal value, stored as a string in reverse order
    void dec_double(std::string& s) const;
    // Increments a decimal value, stored as a string in reverse order
    void dec_inc(std::string& s) const;

    // Shift Helpers 
    void bitwise_sll_const(size_t samt, BitsBase& res) const;
    void bitwise_sxr_const(size_t samt, bool arith, BitsBase& res) const;

    // Trims additional bits down to size_
    void trim();
    // Extends representation to n bits and sign extends if necessary
    void extend_to(size_t n); 
    // Shrinks size and calls trim
    void shrink_to(size_t n);
    // Shrinks to size one and sets val, removes signedness
    void shrink_to_bool(bool b);

    // Returns true if this is a signed number with high order bit set
    bool is_negative() const;

    // Returns the nth (possibly greater than val_.size()th) word of this value.
    // Performs sign extension as necessary.
    T signed_get(size_t n) const;

    // Returns the number of bits in a word
    constexpr size_t bits_per_word() const;
    // Returns the number of bytes in a word
    constexpr size_t bytes_per_word() const;
};

#ifdef __LP64__
using Bits = BitsBase<uint64_t, __uint128_t, int64_t>;
#else
using Bits = BitsBase<uint32_t, uint64_t, int32_t>;
#endif

template <typename T, typename BT, typename ST>
inline BitsBase<T, BT, ST>::BitsBase() {
  val_.push_back(0);
  size_ = 1;
  signed_ = false;
}

template <typename T, typename BT, typename ST>
inline BitsBase<T, BT, ST>::BitsBase(bool b) {
  val_.push_back(b ? T(1) : T(0));
  size_ = 1;
  signed_ = false;
}

template <typename T, typename BT, typename ST>
inline BitsBase<T, BT, ST>::BitsBase(size_t n, T val) : BitsBase() { 
  assert(n > 0);
  extend_to(n);
  val_[0] = val;
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::read(std::istream& is, size_t base) {
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

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::write(std::ostream& os, size_t base) const {
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

template <typename T, typename BT, typename ST>
inline size_t BitsBase<T, BT, ST>::deserialize(std::istream& is) {
  uint32_t header;
  is.read((char*)&header, 4);

  shrink_to_bool(false);
  resize(header & 0x7fffffff);
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

template <typename T, typename BT, typename ST>
inline size_t BitsBase<T, BT, ST>::serialize(std::ostream& os) const {
  const uint32_t header = size_ | (signed_ ? 0x80000000 : 0);
  os.write((char*)&header, 4);

  for (auto v : val_) {
    for (size_t i = 0; i < bytes_per_word(); ++i) {
      uint8_t b = v & 0xff;
      os.write((char*)&b, 1);
      v >>= 8;
    }
  }

  return 4 + val_.size() * bytes_per_word();
}

template <typename T, typename BT, typename ST>
template <typename B>
inline B BitsBase<T, BT, ST>::read_word(size_t n) const {
  assert(sizeof(B) <= sizeof(T));

  // Easy Case:
  if (sizeof(B) == sizeof(T)) {
    assert(n < val_.size());
    return val_[n];
  }

  // Hard Case:
  const auto b_per_word = sizeof(T) / sizeof(B);
  const auto idx = n / b_per_word;
  const auto off = n % b_per_word;

  assert(idx < val_.size());
  return val_[idx] >> (8*sizeof(B)*off);
}

template <typename T, typename BT, typename ST>
template <typename B>
inline void BitsBase<T, BT, ST>::write_word(size_t n, B b) {
  assert(sizeof(B) <= sizeof(T));

  // Easy Case:
  if (sizeof(B) == sizeof(T)) {
    assert(n < val_.size());
    val_[n] = b;
  }

  // Hard Case:
  const auto b_per_word = sizeof(T) / sizeof(B);
  const auto idx = n / b_per_word;
  const auto off = n % b_per_word;

  assert(idx < val_.size());
  const T bmask = B(-1);
  const auto mask = ~(bmask << (8*sizeof(B)*off));
  val_[idx] &= mask;
  val_[idx] |= (T(b) << (8*sizeof(B)*off));
  trim();
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::to_bool() const {
  for (const auto& v : val_) {
    if (v) {
      return true;
    }
  }
  return false;
}

template <typename T, typename BT, typename ST>
inline T BitsBase<T, BT, ST>::to_int() const {
  assert(size_ <= bits_per_word());
  return val_[0];
}

template <typename T, typename BT, typename ST>
inline size_t BitsBase<T, BT, ST>::size() const {
  return size_;
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::resize(size_t n) {
  if (n < size_) {
    shrink_to(n);
  } else if (n > size_) {
    extend_to(n);
  }
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::is_signed() const {
  return signed_;
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::set_signed(bool s) {
  signed_ = s;
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_and(const BitsBase& rhs, BitsBase& res) const {
  assert(size_ == rhs.size_);
  assert(size_ == res.size_);
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    res.val_[i] = val_[i] & rhs.val_[i];
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_or(const BitsBase& rhs, BitsBase& res) const {
  assert(size_ == rhs.size_);
  assert(size_ == res.size_);
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    res.val_[i] = val_[i] | rhs.val_[i];
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_xor(const BitsBase& rhs, BitsBase& res) const {
  assert(size_ == rhs.size_);
  assert(size_ == res.size_);
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    res.val_[i] = val_[i] ^ rhs.val_[i];
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_xnor(const BitsBase& rhs, BitsBase& res) const {
  assert(size_ == rhs.size_);
  assert(size_ == res.size_);
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    res.val_[i] = ~(val_[i] ^ rhs.val_[i]);
  }
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_sll(const BitsBase& rhs, BitsBase& res) const {
  const auto samt = rhs.to_int();
  bitwise_sll_const(samt, res);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_sal(const BitsBase& rhs, BitsBase& res) const {
  // Equivalent to sll
  bitwise_sll(rhs, res);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_slr(const BitsBase& rhs, BitsBase& res) const {
  const auto samt = rhs.to_int();
  bitwise_sxr_const(samt, false, res);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_sar(const BitsBase& rhs, BitsBase& res) const {
  const auto samt = rhs.to_int();
  bitwise_sxr_const(samt, true, res);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_not(BitsBase& res) const {
  assert(size_ == res.size_);
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    res.val_[i] = ~val_[i];
  }
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_plus(BitsBase& res) const {
  assert(size_ == res.size_);
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    res.val_[i] = val_[i];
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_plus(const BitsBase& rhs, BitsBase& res) const {
  assert(size_ == rhs.size_);
  assert(size_ == res.size_);

  T carry = 0;
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    res.val_[i] = val_[i] + rhs.val_[i] + carry;
    if (carry) {
      carry = (res.val_[i] <= val_[i]) ? T(1) : T(0); 
    } else {
      carry = (res.val_[i] < val_[i]) ? T(1) : T(0);
    }
  }
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_minus(BitsBase& res) const {
  assert(size_ == res.size_);

  T carry = 1;
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    res.val_[i] = ~val_[i];
    const T sum = res.val_[i] + carry;
    carry = (sum < res.val_[i]) ? T(1) : T(0);
    res.val_[i] = sum;
  }
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_minus(const BitsBase& rhs, BitsBase& res) const {
  assert(size_ == rhs.size_);
  assert(size_ == res.size_);

  T carry = 0;
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    res.val_[i] = val_[i] - rhs.val_[i] - carry;
    if (carry) {
      carry = (res.val_[i] >= val_[i]) ? T(1) : T(0); 
    } else {
      carry = (res.val_[i] > val_[i]) ? T(1) : T(0);
    }
  }
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_multiply(const BitsBase& rhs, BitsBase& res) const {
  assert(size_ == rhs.size_);
  assert(size_ == res.size_);

  // This is the optimized space algorithm described in wiki's multiplication
  // algorithm article. The code is simplified here, as we can assume that both
  // inputs and the result are capped at S words. 

  const auto S = val_.size();
  BT tot = 0;
  for (size_t ri = 0; ri < S; ++ri) {
    for (size_t bi = 0; bi <= ri; ++bi) {
      size_t ai = ri - bi;
      tot += BT(val_[ai]) * BT(rhs.val_[bi]);
    }
    res.val_[ri] = (T)tot;
    tot >>= bits_per_word();
  }
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_divide(const BitsBase& rhs, BitsBase& res) const {
  // TODO: This only words for single word inputs

  assert(size_ == rhs.size_);
  assert(size_ == res.size_);
  assert(val_.size() == 1);

  if (signed_ && rhs.signed_) {
    const ST l = is_negative() ? (val_[0] | (BT(-1) << size_)) : val_[0];
    const ST r = rhs.is_negative() ? (rhs.val_[0] | (BT(-1) << rhs.size_)) : rhs.val_[0]; 
    res.val_[0] = l / r;
  } else {
    res.val_[0] = val_[0] / rhs.val_[0];
  }

  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_mod(const BitsBase& rhs, BitsBase& res) const {
  // TODO: This only words for single word inputs

  assert(size_ == rhs.size_);
  assert(size_ == res.size_);
  assert(val_.size() == 1);

  if (signed_ && rhs.signed_) {
    const ST l = is_negative() ? (val_[0] | (BT(-1) << size_)) : val_[0];
    const ST r = rhs.is_negative() ? (rhs.val_[0] | (BT(-1) << rhs.size_)) : rhs.val_[0]; 
    res.val_[0] = l % r;
  } else {
    res.val_[0] = val_[0] % rhs.val_[0];
  }

  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_pow(const BitsBase& rhs, BitsBase& res) const {
  // TODO: There's a lot wrong here:
  // 1. We're not respecting verilog semantics (wrt sign and result)
  // 2. This only works for single word inputs

  assert(size_ == rhs.size_);
  assert(size_ == res.size_);
  assert(val_.size() == 1);

  // No resize. This method preserves the bit-width of lhs
  res.val_[0] = std::pow(val_[0], rhs.val_[0]);
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_and(const BitsBase& rhs, BitsBase& res) const {
  res.val_[0] = (to_bool() && rhs.to_bool()) ? T(1) : T(0);
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_or(const BitsBase& rhs, BitsBase& res) const {
  res.val_[0] = (to_bool() || rhs.to_bool()) ? T(1) : T(0);
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_not(BitsBase& res) const {
  res.val_[0] = to_bool() ? T(0) : T(1);
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_eq(const BitsBase& rhs, BitsBase& res) const {
  res.val_[0] = (*this == rhs) ? T(1) : T(0);
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_ne(const BitsBase& rhs, BitsBase& res) const {
  res.val_[0] = (*this != rhs) ? T(1) : T(0);
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_lt(const BitsBase& rhs, BitsBase& res) const {
  res.val_[0] = (*this < rhs) ? T(1) : T(0);
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_lte(const BitsBase& rhs, BitsBase& res) const {
  res.val_[0] = (*this <= rhs) ? T(1) : T(0);
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_gt(const BitsBase& rhs, BitsBase& res) const {
  res.val_[0] = (*this > rhs) ? T(1) : T(0);
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_gte(const BitsBase& rhs, BitsBase& res) const {
  res.val_[0] = (*this >= rhs) ? T(1) : T(0);
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::reduce_and(BitsBase& res) const {
  // Logical operations always yield unsigned results
  for (size_t i = 0, ie = val_.size()-1; i < ie; ++i) {
    if (val_[i] != T(-1)) {
      res.val_[0] = T(0);
      res.trim();
      return;
    }
  }
  const auto mask = (T(1) << (size_ % bits_per_word())) - 1;
  if ((val_.back() & mask) != mask) {
    res.val_[0] = T(0);
    res.trim();
    return; 
  }
  res.val_[0] = T(1);
  res.trim();
  return;
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::reduce_nand(BitsBase& res) const {
  reduce_and(res);
  res.val_[0] = res.val_[0] ? T(0) : T(1);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::reduce_or(BitsBase& res) const {
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    if (val_[i]) {
      res.val_[0] = T(1);
      res.trim();
      return;
    }
  }
  res.val_[0] = T(0);
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::reduce_nor(BitsBase& res) const {
  reduce_or(res);
  res.val_[0] = res.val_[0] ? T(0) : T(1);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::reduce_xor(BitsBase& res) const {
  size_t cnt = 0;
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    cnt += __builtin_popcount(val_[i]);
  }
  res.val_[0] = T(cnt % 2);
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::reduce_xnor(BitsBase& res) const {
  reduce_xor(res);
  res.val_[0] = res.val_[0] ? T(0) : T(1);
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::eq(const BitsBase& rhs) const {
  size_t i = 0;
  for (size_t ie = val_.size()-1; i < ie; ++i) {
    const auto rval = rhs.signed_get(i);
    if (val_[i] != rval) {
      return false;
    }
  }

  const auto rval = rhs.signed_get(i);
  const auto lover = size_ % bits_per_word();
  if (lover == 0) {
    return val_[i] == rval;
  } else {
    const auto mask = (T(1) << lover) - 1;
    return val_[i] == (rval & mask);
  } 
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::eq(size_t idx, const BitsBase& rhs) const {
  assert(idx < size_);
  return get(idx) == rhs.get(0);
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::eq(size_t msb, size_t lsb, const BitsBase& rhs) const {
  // Corner Case: Is this a single bit range?
  if (msb == lsb) {
    return eq(msb, rhs);
  }

  assert(msb < size_);
  assert(msb >= lsb);

  // Compute the size of this slice 
  const auto slice = msb - lsb + 1;
  // How many full words does it span and what's left over at the end?
  const auto span = slice / bits_per_word();
  const auto lover = slice % bits_per_word();
  // Compute indices and offsets
  const auto lower = lsb / bits_per_word();
  const auto loff = lsb % bits_per_word();
  const auto uoff = bits_per_word() - loff;

  // Common Case: Full word comparison
  for (size_t i = 0; i < span; ++i) {
    auto word = (val_[lower+i] >> loff);
    const auto rval = rhs.signed_get(i);
    if (loff > 0) {
      word |= (val_[lower+i+1] << uoff);
    } 
    if (word != rval) {
      return false;
    } 
  }

  // Nothing to do if there are no leftovers:
  if (lover == 0) {
    return true;
  }
  // Edge Case: Compare the remaining bits
  auto word = (val_[lower+span] >> loff);
  const auto rval = rhs.signed_get(span);
  if ((loff > 0) && ((lower+span+1) < val_.size())) {
    word |= (val_[lower+span+1] << uoff);
  } 
  const auto mask = (T(1) << lover) - 1;
  return (word & mask) == (rval & mask);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::assign(const BitsBase& rhs) {
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = rhs.signed_get(i);
  }
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::assign(size_t idx, const BitsBase& rhs) {
  assert(idx < size_);
  set(idx, rhs.get(0));
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::assign(size_t msb, size_t lsb, const BitsBase& rhs) {
  // Corner case: Is this range one bit?
  if (msb == lsb) {
    return assign(msb, rhs);
  }

  assert(msb < size_);
  assert(msb >= lsb);

  // Compute the size of this slice 
  const auto slice = msb - lsb + 1;
  // How many full words does it span and what's left over at the end?
  const auto span = slice / bits_per_word();
  const auto lover = slice % bits_per_word();
  // Compute indices and offsets
  const auto lower = lsb / bits_per_word();
  const auto loff = lsb % bits_per_word();
  const auto uoff = bits_per_word() - loff;
  const auto mask = (T(1) << loff) - 1;

  // Common Case: Copy entire words
  for (size_t i = 0; i < span; ++i) {
    const auto rval = rhs.signed_get(i);
    val_[lower+i] &= mask;
    val_[lower+i] |= (rval << loff);
    if (loff > 0) {
      val_[lower+i+1] &= ~mask;
      val_[lower+i+1] |= (rval >> uoff);
    } 
  }

  // Nothing to do if there are no leftovers
  if (lover == 0) {
    return;
  }
  // Edge Case: Copy the remaining bits
  const auto lmask = (T(1) << lover) - 1;
  const auto rval = rhs.signed_get(span);
  val_[lower+span] &= ~(lmask << loff);
  val_[lower+span] |= ((rval & lmask) << loff);

  if ((lover + loff) > bits_per_word()) {
    const auto delta = lover + loff - bits_per_word();
    const auto hmask = (T(1) << delta) - 1;
    val_[lower+span+1] &= ~hmask;
    val_[lower+span+1] |= ((rval >> (lover-delta)) & hmask);
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::assign(const BitsBase& rhs, size_t idx) {
  val_[0] = idx < rhs.size() ? rhs.get(idx) : T(0);
  for (size_t i = 1, ie = val_.size(); i < ie; ++i) {
    val_[i] = T(0);
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::assign(const BitsBase& rhs, size_t msb, size_t lsb) {
  // Corner Case: Is this range 1 bit?
  if (msb == lsb) {
    return assign(rhs, msb);
  }

  assert(msb < rhs.size_);
  assert(msb >= lsb);

  // How many words does this slice span? Where does it start?
  const auto span = (msb-lsb+bits_per_word()) / bits_per_word();
  const auto base = lsb / bits_per_word();
  const auto bamt = lsb % bits_per_word();
  // Create a mask for extracting the lowest bamt bits from top
  const auto mamt = bits_per_word() - bamt;
  const auto mask = (T(1) << bamt) - 1;

  // Copy as much of rhs as we can (maybe along with a few bits extra)
  size_t i = 0;
  for (size_t ie = std::min(span, val_.size()); i < ie; ++i) {
    if (bamt == 0) {
      val_[i] = rhs.val_[base+i];
    } else {
      const auto top = (base+i+1) < rhs.val_.size() ? rhs.val_[base+i+1] : T(0);
      val_[i] = ((top & mask) << mamt) | (rhs.val_[base+i] >> bamt);
    }
  }
  
  // Zero out anything which is left over
  const auto top = std::min((uint32_t)(msb-lsb+1), size_);
  const auto zword = top / bits_per_word();
  const auto zidx = top % bits_per_word();

  if (zword < val_.size()) {
    val_[zword] &= (T(1) << zidx) - 1;
  }
  for (size_t i = zword+1, ie = val_.size(); i < ie; ++i) {
    val_[i] = T(0);
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::concat(const BitsBase& rhs) {
  bitwise_sll_const(rhs.size_, *this);
  for (size_t i = 0, ie = std::min(val_.size(), rhs.val_.size()); i < ie; ++i) {
    val_[i] |= rhs.val_[i];
  }
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::get(size_t idx) const {
  assert(idx < size_);
  const auto widx = idx / bits_per_word();
  const auto bidx = idx % bits_per_word();
  return val_[widx] & (T(1) << bidx);
}

template <typename T, typename BT, typename ST>
inline BitsBase<T, BT, ST>& BitsBase<T, BT, ST>::set(size_t idx, bool b) {
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

template <typename T, typename BT, typename ST>
inline BitsBase<T, BT, ST>& BitsBase<T, BT, ST>::flip(size_t idx) {
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

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::operator==(const BitsBase& rhs) const {
  assert(size_ == rhs.size_);
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    if (val_[i] != rhs.val_[i]) {
      return false;
    }
  }
  return true;
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::operator!=(const BitsBase& rhs) const {
  return !(*this == rhs);
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::operator<(const BitsBase& rhs) const {
  assert(size_ == rhs.size_);

  if (is_signed() && rhs.is_signed()) {
    const auto lneg = is_negative();
    const auto rneg = rhs.is_negative();
    if (lneg && !rneg) {
      return true; 
    } else if (!lneg && rneg) {
      return false;
    }
  }

  for (int i = val_.size()-1; i >= 0; --i) {
    if (val_[i] < rhs.val_[i]) {
      return true;
    } else if (val_[i] > rhs.val_[i]) {
      return false;
    }
  }
  return false;
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::operator<=(const BitsBase& rhs) const {
  assert(size_ == rhs.size_);

  if (is_signed() && rhs.is_signed()) {
    const auto lneg = is_negative();
    const auto rneg = rhs.is_negative();
    if (lneg && !rneg) {
      return true; 
    } else if (!lneg && rneg) {
      return false;
    }
  }

  for (int i = val_.size()-1; i >= 0; --i) {
    if (val_[i] < rhs.val_[i]) {
      return true;
    } else if (val_[i] > rhs.val_[i]) {
      return false;
    }
  }
  return true;
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::operator>(const BitsBase& rhs) const {
  return !(*this <= rhs);
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::operator>=(const BitsBase& rhs) const {
  return !(*this < rhs);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::read_2_8_16(std::istream& is, size_t base) {
  // Input Buffer:
  std::string s;
  is >> s;

  // Reset internal state
  shrink_to_bool(false);

  // How many bits do we generate per character? Resize internal storage.
  // No need to worry about sign extension; shrink_to_bool reset signedness
  const auto step = (base == 2) ? 1 : (base == 8) ? 3 : 4;
  extend_to(step * s.length());

  // Walk over the buffer from lowest to highest order (that's in reverse)
  size_t word = 0;
  size_t idx = 0;
  for (int i = s.length()-1; i >= 0; --i) {
    // Convert character to bits
    const T bits = isalpha(s[i]) ? (s[i]-'a'+10) : (s[i]-'0');
    // Copy bits into storage and bump idx.
    val_[word] |= (bits << idx);
    idx += step;
    // Keep going if there's still room in this word, otherwise on to the next
    if (idx < bits_per_word()) {
      continue;
    }
    ++word;
    // Easy Case: Step divides words evenly
    if (idx == bits_per_word()) {
      idx = 0;
      continue;
    }
    // Hard Case: Bit overflow
    idx -= bits_per_word();
    val_[word] |= (bits >> (step-idx));
  }

  // Trim trailing 0s (but leave at least one if this number is zero!)
  while ((size_ > 1) && !get(size_-1)) {
    shrink_to(size_-1);
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::read_10(std::istream& is) {
  // Input Buffer:
  std::string s;
  is >> s;

  // Reset interal state
  shrink_to_bool(false);

  // Halve decimal string until it becomes zero. Add 1s when least significant
  // digit is odd, 0s otherwise. No need to worry about sign extension;
  // shrink_to_bool reset signedness
  for (size_t i = 1; !dec_zero(s); ++i) {
    extend_to(i);
    set(i-1, (s.back()-'0') % 2); 
    dec_halve(s);
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::write_2_8_16(std::ostream& os, size_t base) const {
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


template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::write_10(std::ostream& os) const {
  // If this number is negative, switch to positive value
  Bits temp = *this;
  if (temp.is_negative()) {
    os << "-";
    arithmetic_minus(temp);
  }

  // Output Buffer (lsb in index 0):
  std::string buf = "0";
  // Walk binary string from highest to lowest.
  // Double result each time, add 1s as they appear.
  for (int i = size_-1; i >= 0; --i) {
    dec_double(buf);
    if (temp.get(i)) {
      dec_inc(buf);
    }
  }
  // Print the result in reverse
  for (int i = buf.length()-1; i >= 0; --i) {
    os << buf[i];
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::dec_halve(std::string& s) const {
  auto next_carry = 0;
  for (size_t i = 0, ie = s.length(); i < ie; ++i) {
    const auto val = s[i] - '0';
    auto carry = next_carry;
    next_carry = (val % 2) ? 5 : 0;
    s[i] = (val / 2 + carry) + '0';
  }
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::dec_zero(const std::string& s) const {
  for (auto c : s) {
    if (c != '0') {
      return false;
    }
  }
  return true;
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::dec_double(std::string& s) const {
  auto carry = 0;
  for (size_t i = 0, ie = s.size(); i < ie; ++i) {
    auto val = s[i] - '0';
    val = 2 * val + carry;
    if (val >= 10) {
      s[i] = '0' + (val - 10);
      carry = 1;
    } else {
      s[i] = '0' + val;
      carry = 0;
    }
  }
  if (carry) {
    s.push_back('1');
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::dec_inc(std::string& s) const {
  for (size_t i = 0, ie = s.size(); i < ie; ++i) {
    if (++s[i] == ('0'+10)) {
      s[i] = '0';
    } else {
      return;
    }
  }
  s.push_back('1');
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_sll_const(size_t samt, BitsBase& res) const {
  assert(size_ == res.size_);
  
  // Easy Case: We're not actually shifting
  if (samt == 0) {
    for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
      res.val_[i] = val_[i];
    }
    return;
  }

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
      res.val_[w] = val_[b];
    } else {
      res.val_[w] = (val_[b+1] << bamt) | ((val_[b] & mask) >> mamt);
    }
  }
  // There's one more block to build where bottom is implicitly zero
  res.val_[w--] = (bamt == 0) ? T(0) : (val_[0] << bamt);
  // Everything else is zero
  for (; w >= 0; --w) {
    res.val_[w] = T(0);
  }
  // Trim the top and we're done
  res.trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_sxr_const(size_t samt, bool arith, BitsBase& res) const {
  assert(size_ == res.size_);

  // Easy Case: We're not actually shifting
  if (samt == 0) {
    for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
      res.val_[i] = val_[i];
    }
    return;
  }

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
      res.val_[w] = val_[t];
    } else {
      res.val_[w] = (val_[t-1] >> bamt) | ((val_[t] & mask) << mamt);
    }
  }
  // There's one more block to build where top is implicitly zero
  if (hob) {
    res.val_[w++] = (bamt == 0) ? T(-1) : ((val_.back() >> bamt) | (mask << mamt));
  } else {
    res.val_[w++] = (bamt == 0) ? T(0) : (val_.back() >> bamt);
  }
  // Everything else is zero or padded 1s
  for (size_t we = val_.size(); w < we; ++w) {
    res.val_[w] = hob ? T(-1) : T(0);
  }
  // Trim since we could have introduced trailing 1s
  res.trim();
}

template <typename T, typename BT, typename ST>
void BitsBase<T, BT, ST>::trim() {
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
    
template <typename T, typename BT, typename ST>
void BitsBase<T, BT, ST>::extend_to(size_t n) {
  const auto words = (n + bits_per_word() - 1) / bits_per_word();
  if (is_negative()) {
    val_.back() |= (T(-1) << (size_ % bits_per_word()));
    val_.resize(words, T(-1));
    size_ = n;
    trim();
  } else {
    val_.resize(words, T(0));
    size_ = n;
  }
}
   
template <typename T, typename BT, typename ST>
void BitsBase<T, BT, ST>::shrink_to(size_t n) {
  const auto words = (n + bits_per_word() - 1) / bits_per_word();
  val_.resize(words, T(0));
  size_ = n;
  trim();
}

template <typename T, typename BT, typename ST>
void BitsBase<T, BT, ST>::shrink_to_bool(bool b) {
  val_.resize(1, T(0));
  val_[0] = b ? 1 : 0;
  size_ = 1;
  signed_ = false;
}

template <typename T, typename BT, typename ST>
bool BitsBase<T, BT, ST>::is_negative() const {
  return signed_ && get(size_-1);
}

template <typename T, typename BT, typename ST>
T BitsBase<T, BT, ST>::signed_get(size_t n) const {
  // Difficult case: Do we need to sign extend this value?
  if (n == val_.size()-1) {
    const auto top = size_ % bits_per_word();
    if (!is_negative() || (top == 0)) {
      return val_[n];
    }
    return val_[n] | (T(-1) << top);
  }
  // Easier Case: Do we need to return all 1s or 0s?
  else if (n >= val_.size()) {
    return is_negative() ? T(-1) : T(0);
  }
  // Easiest Case: Just return what's there
  else {
    return val_[n]; 
  }
}

template <typename T, typename BT, typename ST>
constexpr size_t BitsBase<T, BT, ST>::bits_per_word() const {
  return 8 * bytes_per_word();
}

template <typename T, typename BT, typename ST>
constexpr size_t BitsBase<T, BT, ST>::bytes_per_word() const {
  return sizeof(T);
}

} // namespace cascade

#endif
