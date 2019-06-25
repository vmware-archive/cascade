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
#include "base/container/vector.h"
#include "base/serial/serializable.h"

namespace cascade {

// This class is the fundamental representation of a bit string. 

template <typename T, typename BT, typename ST>
class BitsBase : public Serializable {
  public:
    // Supporting Concepts:
    enum class Type : uint8_t {
      UNSIGNED = 0,
      SIGNED,
      REAL
    };

    // Constructors:
    BitsBase();
    explicit BitsBase(size_t n, Type t);
    explicit BitsBase(bool b);
    explicit BitsBase(char c);
    explicit BitsBase(double d);
    explicit BitsBase(const std::string& s);
    explicit BitsBase(size_t n, T val);

    // Compiler-Generated Constructors:
    BitsBase(const BitsBase& rhs) = default;
    BitsBase(BitsBase&& rhs) = default;
    BitsBase& operator=(const BitsBase& rhs) = default;
    BitsBase& operator=(BitsBase&& rhs) = default;
    ~BitsBase() override = default;
    
    // Serial I/O:
    void read(std::istream& is, size_t base);
    void write(std::ostream& os, size_t base) const;
    size_t deserialize(std::istream& is) override;
    size_t serialize(std::ostream& os) const override;

    // Block I/O:
    template <typename B>
    B read_word(size_t n) const;
    template <typename B>
    void write_word(size_t n, B b);

    // Type Introspection:
    size_t size() const;
    Type get_type() const;
    bool is_signed() const;
    bool is_real() const;

    // Native Casts:
    bool to_bool() const;
    char to_char() const;
    double to_double() const;
    std::string to_string() const;
    T to_uint() const;

    // Verilog Casts:
    void resize(size_t n);
    void cast_type(Type t);
    void reinterpret_type(Type t);

    // Bitwise Operators: 
    //
    // Apply a bitwise operator to operands and store the result here.  These
    // methods assume equivalent bit-width between operands and destination and
    // do not sign extend. These methods are undefined for reals.
    void bitwise_and(const BitsBase& lhs, const BitsBase& rhs);
    void bitwise_or(const BitsBase& lhs, const BitsBase& rhs);
    void bitwise_xor(const BitsBase& lhs, const BitsBase& rhs);
    void bitwise_xnor(const BitsBase& lhs, const BitsBase& rhs);
    void bitwise_sll(const BitsBase& lhs, const BitsBase& rhs);
    void bitwise_sal(const BitsBase& lhs, const BitsBase& rhs);
    void bitwise_slr(const BitsBase& lhs, const BitsBase& rhs);
    void bitwise_sar(const BitsBase& lhs, const BitsBase& rhs);
    void bitwise_not(const BitsBase& lhs);

    // Arithmetic Operators:
    //
    // Apply an arithmetic operator to operands and store the result here.
    // These methods all assume equivalent bit-width between operands and
    // destination and do not sign extend. These methods all work on signed,
    // unsigned, and real values (with the exception of mod).
    void arithmetic_plus(const BitsBase& lhs);
    void arithmetic_plus(const BitsBase& lhs, const BitsBase& rhs);
    void arithmetic_minus(const BitsBase& lhs);
    void arithmetic_minus(const BitsBase& lhs, const BitsBase& rhs);
    void arithmetic_multiply(const BitsBase& lhs, const BitsBase& rhs);
    void arithmetic_divide(const BitsBase& lhs, const BitsBase& rhs);
    void arithmetic_mod(const BitsBase& lhs, const BitsBase& rhs);
    void arithmetic_pow(const BitsBase& lhs, const BitsBase& rhs);

    // Logical Operators:
    //
    // Apply a logical operator to operands and store the result here.  These
    // methods all assume equivalent bit-width between operands and do not
    // perform sign extension.  These methods all work on signed, unsigned, and
    // real values, but assume that their desintation is not real.
    void logical_and(const BitsBase& lhs, const BitsBase& rhs);
    void logical_or(const BitsBase& lhs, const BitsBase& rhs);
    void logical_not(const BitsBase& lhs);
    void logical_eq(const BitsBase& lhs, const BitsBase& rhs);
    void logical_ne(const BitsBase& lhs, const BitsBase& rhs);
    void logical_lt(const BitsBase& lhs, const BitsBase& rhs);
    void logical_lte(const BitsBase& lhs, const BitsBase& rhs);
    void logical_gt(const BitsBase& lhs, const BitsBase& rhs);
    void logical_gte(const BitsBase& lhs, const BitsBase& rhs);

    // Reduction Operators:
    //
    // Apply a reduction operator to an operand and store the result here.
    // These methods all assume single bit-width in their destination and do
    // not perform sign extension. These methods are undefined for real values.
    void reduce_and(const BitsBase& lhs);
    void reduce_nand(const BitsBase& lhs);
    void reduce_or(const BitsBase& lhs);
    void reduce_nor(const BitsBase& lhs);
    void reduce_xor(const BitsBase& lhs);
    void reduce_xnor(const BitsBase& lhs);

    // Comparison Operators:
    //
    // Check for value equality. These methods make no assumptions with respect
    // to size or type and cast rhs as necessary.
    bool eq(const BitsBase& rhs) const;
    bool eq(size_t idx, const BitsBase& rhs) const;
    bool eq(size_t msb, size_t lsb, const BitsBase& rhs) const;

    // Assignment Operators:
    //
    // Assign bits. These methods make no assumptions made with respect to
    // sizes and handle sign-extension and type conversion as necessary. 
    // Subscripting operations are undefined for real operands.
    void assign(const BitsBase& rhs);
    void assign(size_t idx, const BitsBase& rhs);
    void assign(size_t msb, size_t lsb, const BitsBase& rhs);
    void assign(const BitsBase& rhs, size_t idx);
    void assign(const BitsBase& rhs, size_t msb, size_t lsb);

    // Concatenation Operations:
    //
    // Concatenate two variables. This method is undefined for real values.
    void concat(const BitsBase& rhs);

    // Bitwise Operators:
    //
    // These operators are meant for use outside of the constraints imposed by
    // the verilog semantics for variables and have no restrictions on type.
    bool get(size_t idx) const;
    void set(size_t idx, bool b);
    void flip(size_t idx);

    // Built-in Comparison Operators:
    //
    // Logical comparison, assumes equal operand sizes and types
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
    Type type_;

    // Reads a real number, sets size to 64
    void read_real(std::istream& is);
    // Reads an unsigned number in base 2, 8, or 16, sets size based on result 
    void read_2_8_16(std::istream& is, size_t base);
    // Reads a number in base 10, sets size and type based on result 
    void read_10(std::istream& is);
    // Writes a number as a real
    void write_real(std::ostream& os) const;
    // Writes a number in base 2, 8, or 16 as an unsigned value
    void write_2_8_16(std::ostream& os, size_t base) const;
    // Writes a number in base 10 as a signed or unsigned value
    void write_10(std::ostream& os) const;
    // Halves a decimal value, stored as a string
    void dec_halve(std::string& s) const;
    // Returns true if a decimal value, stored as a string is 0
    bool dec_zero(const std::string& s) const;
    // Doubles a decimal value, stored as a string in reverse order
    void dec_double(std::string& s) const;
    // Increments a decimal value, stored as a string in reverse order
    void dec_inc(std::string& s) const;

    // Shift Helpers:
    void bitwise_sll_const(const BitsBase& lhs, size_t samt);
    void bitwise_sxr_const(const BitsBase& lhs, size_t samt, bool arith);

    // Returns the nth (possibly greater than val_.size()th) word of this value.
    // Performs sign extension as necessary.
    T signed_get(size_t n) const;

    // Returns true if this is a signed number with high order bit set
    bool is_neg_signed() const;
    // Zeros out bits with index greater than size_ 
    void trim();
    // Extends representation to n bits and zero pads
    void extend_to(size_t n); 
    // Extends representation to n bits and sign extends if necessary
    void sign_extend_to(size_t n); 
    // Shrinks size and calls trim
    void shrink_to(size_t n);
    // Shrinks to size one and sets val
    void shrink_to_bool(bool b);
    // Inverts bits and adds one
    void invert_add_one();
    // Updates size and type; sets value according to to_double().
    void cast_int_to_real();
    // Updates type according to arg, value and size according to_double().
    void cast_real_to_int(bool s);

    // Returns the number of bits in a word
    constexpr size_t bits_per_word() const;
    // Returns the number of bytes in a word
    constexpr size_t bytes_per_word() const;
    // Returns the number of unique values representable by T as a double
    constexpr double range() const;
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
  type_ = Type::UNSIGNED;
}

template <typename T, typename BT, typename ST>
inline BitsBase<T, BT, ST>::BitsBase(size_t n, Type t) {
  if (type_ == Type::REAL) {
    assert(n == 64);
    val_.resize(64/bits_per_word());
    *reinterpret_cast<double*>(val_.data()) = 0.0;
  } else {
    assert(n > 0);
    val_.resize((n+bits_per_word()-1)/bits_per_word(), static_cast<T>(0));
  }
  size_ = n;
  type_ = t;
}

template <typename T, typename BT, typename ST>
inline BitsBase<T, BT, ST>::BitsBase(bool b) {
  val_.push_back(b ? static_cast<T>(1) : static_cast<T>(0));
  size_ = 1;
  type_ = Type::UNSIGNED;
}

template <typename T, typename BT, typename ST>
inline BitsBase<T, BT, ST>::BitsBase(char c) {
  val_.push_back(static_cast<T>(c));
  size_ = 8;
  type_ = Type::UNSIGNED;
}

template <typename T, typename BT, typename ST>
inline BitsBase<T, BT, ST>::BitsBase(double d) {
  val_.resize(64/bits_per_word());
  *reinterpret_cast<double*>(val_.data()) = d;
  size_ = 64;
  type_ = Type::REAL;
}

template <typename T, typename BT, typename ST>
inline BitsBase<T, BT, ST>::BitsBase(const std::string& s) {
  val_.resize((s.length()+bytes_per_word()-1)/bytes_per_word(), static_cast<T>(0));
  for (int pos = 0, i = s.length()-1; i >= 0; --i, ++pos) {
    const auto idx = pos/bytes_per_word();
    const auto off = 8*(pos%bytes_per_word()); 
    val_[idx] |= (static_cast<T>(s[i]) << off);
  }
  size_ = 8*s.length();
  type_ = Type::UNSIGNED;
}

template <typename T, typename BT, typename ST>
inline BitsBase<T, BT, ST>::BitsBase(size_t n, T val) : BitsBase() { 
  assert(n > 0);
  val_.resize((n+bits_per_word()-1)/bits_per_word(), static_cast<T>(0));
  val_[0] = val;
  size_ = n;
  type_ = Type::UNSIGNED;
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::read(std::istream& is, size_t base) {
  switch (base) {
    case 1:
      return read_real(is);
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
    case 0:
      return is_real() ? write_real(os) : write_10(os);
    case 1:
      return write_real(os);
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
  is.read(reinterpret_cast<char*>(&header), 4);

  shrink_to_bool(false);
  extend_to(header & 0x3fffffffu);
  type_ = static_cast<Type>(header >> 30);

  const auto n = (size_+7) / 8;
  for (size_t i = 0; i < n; ++i) {
    uint8_t b = is.get();
    val_[i/bytes_per_word()] |= (static_cast<T>(b) << (8*(i%bytes_per_word())));
  }

  return 4 + n;
}

template <typename T, typename BT, typename ST>
inline size_t BitsBase<T, BT, ST>::serialize(std::ostream& os) const {
  uint32_t header = size_ | (static_cast<uint32_t>(type_) << 30);
  os.write(reinterpret_cast<char*>(&header), 4);

  const auto n = (size_+7) / 8;
  for (size_t i = 0; i < n; ++i) {
    const uint8_t b = (val_[i/bytes_per_word()] >> (8*(i%bytes_per_word()))) & static_cast<T>(0xffu);
    os.put(b);
  }

  return 4 + n;
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
    return;
  }

  // Hard Case:
  const auto b_per_word = sizeof(T) / sizeof(B);
  const auto idx = n / b_per_word;
  const auto off = n % b_per_word;

  assert(idx < val_.size());
  const T bmask = static_cast<B>(-1);
  const auto mask = ~(bmask << (8*sizeof(B)*off));
  val_[idx] &= mask;
  val_[idx] |= (static_cast<T>(b) << (8*sizeof(B)*off));
  trim();
}

template <typename T, typename BT, typename ST>
inline size_t BitsBase<T, BT, ST>::size() const {
  return size_;
}

template <typename T, typename BT, typename ST>
inline typename BitsBase<T, BT, ST>::Type BitsBase<T, BT, ST>::get_type() const {
  return type_;
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::is_signed() const {
  return type_ != Type::UNSIGNED;
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::is_real() const {
  return type_ == Type::REAL;
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::to_bool() const {
  // Special handling for real values.
  if (type_ == Type::REAL) {
    return *reinterpret_cast<const double*>(val_.data()) != 0.0;
  }
  // Otherwise check whether any bits are non-zero.
  for (const auto& v : val_) {
    if (v) {
      return true;
    }
  }
  return false;
}

template <typename T, typename BT, typename ST>
inline char BitsBase<T, BT, ST>::to_char() const {
  return static_cast<char>(to_uint() & 0xffu);
}

template <typename T, typename BT, typename ST>
inline double BitsBase<T, BT, ST>::to_double() const {
  switch (type_) {
    case Type::SIGNED:
      if (get(size_-1)) {
        const_cast<BitsBase*>(this)->invert_add_one();
        const_cast<BitsBase*>(this)->type_ = Type::UNSIGNED;
        const auto res = to_double();
        const_cast<BitsBase*>(this)->type_ = Type::SIGNED;
        const_cast<BitsBase*>(this)->invert_add_one();
        return res;
      } 
      // Fallthrough...
    case Type::UNSIGNED: {
      double res = 0.0;
      double base = 1.0;
      for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
        res += base * val_[i];
        base *= range();
      }
      return res;
    }
    case Type::REAL:
      return *reinterpret_cast<const double*>(val_.data());
    default:
      assert(false);
      return 0;
  }
}

template <typename T, typename BT, typename ST>
inline std::string BitsBase<T, BT, ST>::to_string() const {
  auto temp = *this;
  // Cast real values to unsigned ints before printing.
  if (type_ == Type::REAL) {
    temp.cast_real_to_int(false);
  }
  // Pad values out to a multiple of 8 bits.
  const auto trailing = temp.size() % 8;
  if (trailing > 0) {
    temp.sign_extend_to(temp.size() + 8 - trailing);
  }
  // Print characters, eight bits at a time.
  std::string res(temp.size()/8, ' ');
  for (int pos = 0, i = res.length()-1; i >= 0; --i, ++pos) {
    const auto idx = pos/bytes_per_word();
    const auto off = 8*(pos%bytes_per_word()); 
    const auto val = (temp.val_[idx] >> off) & static_cast<T>(0xffu);
    res[i] = ((val == 0) ? ' ' : static_cast<char>(val));
  }
  return res;
}

template <typename T, typename BT, typename ST>
inline T BitsBase<T, BT, ST>::to_uint() const {
  switch (type_) {
    case Type::UNSIGNED: 
    case Type::SIGNED: 
      return val_[0];
    case Type::REAL:
      return std::round(*reinterpret_cast<const double*>(val_.data()));
    default:
      assert(false);
      return 0;
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::resize(size_t n) {
  assert((type_ != Type::REAL) || (n == 64));
  if (n < size_) {
    shrink_to(n);
  } else if (n > size_) {
    sign_extend_to(n);
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::cast_type(Type t) {
  if (type_ == t) {
    return;
  } else if (t == Type::REAL) {
    cast_int_to_real();
  } else if (type_ == Type::REAL) {
    cast_real_to_int(t == Type::SIGNED);
  } else {
    type_ = t;
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::reinterpret_type(Type t) {
  if (type_ == t) {
    return;
  } 
  type_ = t;
  if (type_ == Type::REAL) {
    val_.resize(64/bits_per_word());
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_and(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!is_real() && !lhs.is_real() && !rhs.is_real());
  assert(size() == lhs.size());
  assert(size() == rhs.size());

  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = lhs.val_[i] & rhs.val_[i];
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_or(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!is_real() && !lhs.is_real() && !rhs.is_real());
  assert(size() == lhs.size());
  assert(size() == rhs.size());

  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = lhs.val_[i] | rhs.val_[i];
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_xor(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!is_real() && !lhs.is_real() && !rhs.is_real());
  assert(size() == lhs.size());
  assert(size() == rhs.size());

  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = lhs.val_[i] ^ rhs.val_[i];
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_xnor(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!is_real() && !lhs.is_real() && !rhs.is_real());
  assert(size() == lhs.size());
  assert(size() == rhs.size());

  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = ~(lhs.val_[i] ^ rhs.val_[i]);
  }
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_sll(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!rhs.is_real());
  const auto samt = rhs.to_uint();
  bitwise_sll_const(lhs, samt);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_sal(const BitsBase& lhs, const BitsBase& rhs) {
  // Equivalent to sll
  bitwise_sll(lhs, rhs);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_slr(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!rhs.is_real());
  const auto samt = rhs.to_uint();
  bitwise_sxr_const(lhs, samt, false);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_sar(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!rhs.is_real());
  const auto samt = rhs.to_uint();
  bitwise_sxr_const(lhs, samt, true);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_not(const BitsBase& lhs) {
  assert(!is_real() && !lhs.is_real());
  assert(size() == lhs.size());

  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = ~lhs.val_[i];
  }
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_plus(const BitsBase& lhs) {
  // This is a copy; identical implementation for reals and integers
  assert(size() == lhs.size());
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = lhs.val_[i];
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_plus(const BitsBase& lhs, const BitsBase& rhs) {
  if (lhs.is_real() || rhs.is_real()) {
    assert(size() == 64);
    *reinterpret_cast<double*>(val_.data()) = lhs.to_double() + rhs.to_double();
    return;
  }

  assert(size() == lhs.size());
  assert(size() == rhs.size());

  T carry = 0;
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = lhs.val_[i] + rhs.val_[i] + carry;
    if (carry) {
      carry = (val_[i] <= lhs.val_[i]) ? static_cast<T>(1) : static_cast<T>(0); 
    } else {
      carry = (val_[i] < lhs.val_[i]) ? static_cast<T>(1) : static_cast<T>(0);
    }
  }
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_minus(const BitsBase& lhs) {
  if (lhs.is_real()) {
    assert(size() == 64);
    *reinterpret_cast<double*>(val_.data()) = -lhs.to_double();
    return;
  }

  assert(size() == lhs.size());

  T carry = 1;
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = ~lhs.val_[i];
    const T sum = val_[i] + carry;
    carry = (sum < val_[i]) ? static_cast<T>(1) : static_cast<T>(0);
    val_[i] = sum;
  }
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_minus(const BitsBase& lhs, const BitsBase& rhs) {
  if (lhs.is_real() || rhs.is_real()) {
    assert(size() == 64);
    *reinterpret_cast<double*>(val_.data()) = lhs.to_double() - rhs.to_double();
    return;
  }

  assert(size() == lhs.size());
  assert(size() == rhs.size());

  T carry = 0;
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = lhs.val_[i] - rhs.val_[i] - carry;
    if (carry) {
      carry = (val_[i] >= lhs.val_[i]) ? static_cast<T>(1) : static_cast<T>(0); 
    } else {
      carry = (val_[i] > lhs.val_[i]) ? static_cast<T>(1) : static_cast<T>(0);
    }
  }
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_multiply(const BitsBase& lhs, const BitsBase& rhs) {
  if (lhs.is_real() || rhs.is_real()) {
    assert(size() == 64);
    *reinterpret_cast<double*>(val_.data()) = lhs.to_double() * rhs.to_double();
    return;
  }

  assert(size() == lhs.size());
  assert(size() == rhs.size());

  // This is the optimized space algorithm described in wiki's multiplication
  // algorithm article. The code is simplified here, as we can assume that both
  // inputs and the result are capped at S words. 

  const auto S = val_.size();
  BT tot = 0;
  for (size_t ri = 0; ri < S; ++ri) {
    for (size_t bi = 0; bi <= ri; ++bi) {
      size_t ai = ri - bi;
      tot += static_cast<BT>(lhs.val_[ai]) * static_cast<BT>(rhs.val_[bi]);
    }
    val_[ri] = static_cast<T>(tot);
    tot >>= bits_per_word();
  }
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_divide(const BitsBase& lhs, const BitsBase& rhs) {
  if (lhs.is_real() || rhs.is_real()) {
    assert(size() == 64);
    *reinterpret_cast<double*>(val_.data()) = lhs.to_double() / rhs.to_double();
    return;
  }

  assert(size() == lhs.size());
  assert(size() == rhs.size());

  // TODO(eschkufz) This only words for single word inputs
  assert(val_.size() == 1);

  if ((lhs.type_ == Type::SIGNED) && (rhs.type_ == Type::SIGNED)) {
    const ST l = lhs.is_neg_signed() ? (lhs.val_[0] | (static_cast<BT>(-1) << size_)) : lhs.val_[0];
    const ST r = rhs.is_neg_signed() ? (rhs.val_[0] | (static_cast<BT>(-1) << rhs.size_)) : rhs.val_[0]; 
    val_[0] = l / r;
  } else {
    val_[0] = lhs.val_[0] / rhs.val_[0];
  }
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_mod(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!lhs.is_real() && !rhs.is_real());
  assert(size() == lhs.size());
  assert(size() == rhs.size());

  // TODO(eschkufz) This only words for single word inputs
  assert(val_.size() == 1);

  if ((lhs.type_ == Type::SIGNED) && (rhs.type_ == Type::SIGNED)) {
    const ST l = lhs.is_neg_signed() ? (lhs.val_[0] | (static_cast<BT>(-1) << size_)) : lhs.val_[0];
    const ST r = rhs.is_neg_signed() ? (rhs.val_[0] | (static_cast<BT>(-1) << rhs.size_)) : rhs.val_[0]; 
    val_[0] = l % r;
  } else {
    val_[0] = lhs.val_[0] % rhs.val_[0];
  }
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::arithmetic_pow(const BitsBase& lhs, const BitsBase& rhs) {
  // TODO(eschkufz) There's a lot wrong here:
  // 1. We're not respecting verilog semantics (wrt sign and result)
  // 2. This only works for single word inputs
  // 3. We're not supporting reals, which this should be defined for

  assert(!lhs.is_real() && !rhs.is_real());
  assert(size() == lhs.size());
  assert(size() == rhs.size());
  assert(val_.size() == 1);

  // No resize. This method preserves the bit-width of lhs
  val_[0] = std::pow(lhs.val_[0], rhs.val_[0]);
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_and(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!is_real());
  val_[0] = (lhs.to_bool() && rhs.to_bool()) ? static_cast<T>(1) : static_cast<T>(0);
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_or(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!is_real());
  val_[0] = (lhs.to_bool() || rhs.to_bool()) ? static_cast<T>(1) : static_cast<T>(0);
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_not(const BitsBase& lhs) {
  assert(!is_real());
  val_[0] = lhs.to_bool() ? static_cast<T>(0) : static_cast<T>(1);
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_eq(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!is_real());
  val_[0] = (lhs == rhs) ? static_cast<T>(1) : static_cast<T>(0);
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_ne(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!is_real());
  val_[0] = (lhs != rhs) ? static_cast<T>(1) : static_cast<T>(0);
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_lt(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!is_real());
  val_[0] = (lhs < rhs) ? static_cast<T>(1) : static_cast<T>(0);
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_lte(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!is_real());
  val_[0] = (lhs <= rhs) ? static_cast<T>(1) : static_cast<T>(0);
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_gt(const BitsBase& lhs, const BitsBase& rhs) {
  assert(!is_real());
  val_[0] = (lhs > rhs) ? static_cast<T>(1) : static_cast<T>(0);
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::logical_gte(const BitsBase& lhs, const BitsBase& rhs){
  assert(!is_real());
  val_[0] = (lhs >= rhs) ? static_cast<T>(1) : static_cast<T>(0);
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::reduce_and(const BitsBase& lhs) {
  assert(!is_real() && !lhs.is_real());
  // Logical operations always yield unsigned results
  for (size_t i = 0, ie = lhs.val_.size()-1; i < ie; ++i) {
    if (lhs.val_[i] != static_cast<T>(-1)) {
      val_[0] = static_cast<T>(0);
      trim();
      return;
    }
  }
  const auto mask = (static_cast<T>(1) << (size_ % bits_per_word())) - 1;
  if ((lhs.val_.back() & mask) != mask) {
    val_[0] = static_cast<T>(0);
    trim();
    return; 
  }
  val_[0] = static_cast<T>(1);
  trim();
  return;
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::reduce_nand(const BitsBase& lhs) {
  assert(!is_real() && !lhs.is_real());
  reduce_and(lhs);
  val_[0] = (val_[0] != 0) ? static_cast<T>(0) : static_cast<T>(1);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::reduce_or(const BitsBase& lhs) {
  assert(!is_real() && !lhs.is_real());
  for (size_t i = 0, ie = lhs.val_.size(); i < ie; ++i) {
    if (lhs.val_[i]) {
      val_[0] = static_cast<T>(1);
      trim();
      return;
    }
  }
  val_[0] = static_cast<T>(0);
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::reduce_nor(const BitsBase& lhs) {
  assert(!is_real() && !lhs.is_real());
  reduce_or(lhs);
  val_[0] = (val_[0] != 0) ? static_cast<T>(0) : static_cast<T>(1);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::reduce_xor(const BitsBase& lhs) {
  assert(!is_real() && !lhs.is_real());
  size_t cnt = 0;
  for (size_t i = 0, ie = lhs.val_.size(); i < ie; ++i) {
    cnt += __builtin_popcount(lhs.val_[i]);
  }
  val_[0] = static_cast<T>(cnt % 2);
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::reduce_xnor(const BitsBase& lhs) {
  assert(!is_real() && !lhs.is_real());
  reduce_xor(lhs);
  val_[0] = (val_[0] != 0) ? static_cast<T>(0) : static_cast<T>(1);
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::eq(const BitsBase& rhs) const {
  if (is_real() && rhs.is_real()) {
    return *reinterpret_cast<const double*>(val_.data()) == *reinterpret_cast<const double*>(rhs.val_.data());
  } 
  if (is_real()) {
    auto temp = *this;
    temp.cast_type(Type::SIGNED);
    return temp.eq(rhs);
  }
  if (rhs.is_real()) {
    auto temp = rhs;
    temp.cast_type(Type::SIGNED);
    return eq(temp);
  }

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
    const auto mask = (static_cast<T>(1) << lover) - 1;
    return val_[i] == (rval & mask);
  } 
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::eq(size_t idx, const BitsBase& rhs) const {
  assert(!is_real());
  if (rhs.is_real()) {
    auto temp = rhs;
    temp.cast_type(Type::SIGNED);
    return eq(idx, temp);
  }

  assert(idx < size_);
  return get(idx) == rhs.get(0);
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::eq(size_t msb, size_t lsb, const BitsBase& rhs) const {
  assert(!is_real());
  if (rhs.is_real()) {
    auto temp = rhs;
    temp.cast_type(Type::SIGNED);
    return eq(msb, lsb, temp);
  }

  // Corner Case: Is this a single bit range?
  if (msb == lsb) {
    return eq(msb, rhs);
  }

  assert(msb < size_);
  assert(msb >= lsb);

  // Compute the size of this slice 
  const auto slice = (msb - lsb) + 1;
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
  const auto mask = (static_cast<T>(1) << lover) - 1;
  return (word & mask) == (rval & mask);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::assign(const BitsBase& rhs) {
  if (is_real()) {
    const auto val = rhs.is_real() ? *reinterpret_cast<const double*>(rhs.val_.data()) : rhs.to_double();
    *reinterpret_cast<double*>(val_.data()) = val;
    return;
  } 
  if (rhs.is_real()) {
    auto temp = rhs;
    temp.cast_real_to_int(false);
    return assign(temp);
  }

  assert(!is_real());
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = rhs.signed_get(i);
  }
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::assign(size_t idx, const BitsBase& rhs) {
  assert(!is_real());
  assert(idx < size_);
  set(idx, rhs.to_uint() & static_cast<T>(1));
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::assign(size_t msb, size_t lsb, const BitsBase& rhs) {
  // Corner Case: Is this range one bit?
  if (msb == lsb) {
    return assign(msb, rhs);
  }
  // Corner Case: Do we need to convert rhs to an integer?
  if (rhs.is_real()) {
    auto temp = rhs;
    temp.cast_real_to_int(false);
    return assign(msb, lsb, temp);
  }

  assert(!is_real() && !rhs.is_real());
  assert(msb < size_);
  assert(msb >= lsb);

  // Compute the size of this slice 
  const auto slice = (msb - lsb) + 1;
  // How many full words does it span and what's left over at the end?
  const auto span = slice / bits_per_word();
  const auto lover = slice % bits_per_word();
  // Compute indices and offsets
  const auto lower = lsb / bits_per_word();
  const auto loff = lsb % bits_per_word();
  const auto uoff = bits_per_word() - loff;
  const auto mask = (static_cast<T>(1) << loff) - 1;

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
  const auto lmask = (static_cast<T>(1) << lover) - 1;
  const auto rval = rhs.signed_get(span);
  val_[lower+span] &= ~(lmask << loff);
  val_[lower+span] |= ((rval & lmask) << loff);

  if ((lover + loff) > bits_per_word()) {
    const auto delta = (lover + loff) - bits_per_word();
    const auto hmask = (static_cast<T>(1) << delta) - 1;
    val_[lower+span+1] &= ~hmask;
    val_[lower+span+1] |= ((rval >> (lover-delta)) & hmask);
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::assign(const BitsBase& rhs, size_t idx) {
  val_[0] = (idx < rhs.size()) ? rhs.get(idx) : static_cast<T>(0);
  for (size_t i = 1, ie = val_.size(); i < ie; ++i) {
    val_[i] = static_cast<T>(0);
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
  const auto span = ((msb-lsb)+bits_per_word()) / bits_per_word();
  const auto base = lsb / bits_per_word();
  const auto bamt = lsb % bits_per_word();
  // Create a mask for extracting the lowest bamt bits from top
  const auto mamt = bits_per_word() - bamt;
  const auto mask = (static_cast<T>(1) << bamt) - 1;

  // Copy as much of rhs as we can (maybe along with a few bits extra)
  size_t i = 0;
  for (size_t ie = std::min(span, val_.size()); i < ie; ++i) {
    if (bamt == 0) {
      val_[i] = rhs.val_[base+i];
    } else {
      const auto top = ((base+i+1) < rhs.val_.size()) ? rhs.val_[base+i+1] : static_cast<T>(0);
      val_[i] = ((top & mask) << mamt) | (rhs.val_[base+i] >> bamt);
    }
  }
  
  // Zero out anything which is left over
  const auto top = std::min(static_cast<uint32_t>((msb-lsb)+1), size_);
  const auto zword = top / bits_per_word();
  const auto zidx = top % bits_per_word();

  if (zword < val_.size()) {
    val_[zword] &= (static_cast<T>(1) << zidx) - 1;
  }
  for (size_t i = zword+1, ie = val_.size(); i < ie; ++i) {
    val_[i] = static_cast<T>(0);
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::concat(const BitsBase& rhs) {
  assert(!is_real() && !rhs.is_real());

  bitwise_sll_const(*this, rhs.size_);
  for (size_t i = 0, ie = std::min(val_.size(), rhs.val_.size()); i < ie; ++i) {
    val_[i] |= rhs.val_[i];
  }
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::get(size_t idx) const {
  assert(idx < size_);

  const auto widx = idx / bits_per_word();
  const auto bidx = idx % bits_per_word();
  return val_[widx] & (static_cast<T>(1) << bidx);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::set(size_t idx, bool b) {
  assert(idx < size_);

  const auto widx = idx / bits_per_word();
  const auto bidx = idx % bits_per_word();
  if (b) {
    val_[widx] |= (static_cast<T>(1) << bidx);
  } else {
    val_[widx] &= ~(static_cast<T>(1) << bidx);
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::flip(size_t idx) {
  assert(idx < size_);

  const auto widx = idx / bits_per_word();
  const auto bidx = idx % bits_per_word();
  const bool b = (val_[widx] >> bidx) & static_cast<T>(1);
  if (!b) {
    val_[widx] |= (static_cast<T>(1) << bidx);
  } else {
    val_[widx] &= ~(static_cast<T>(1) << bidx);
  }
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

  if ((type_ == Type::SIGNED) && (rhs.type_ == Type::SIGNED)) {
    const auto lneg = is_neg_signed();
    const auto rneg = rhs.is_neg_signed();
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

  if ((type_ == Type::SIGNED) && (rhs.type_ == Type::SIGNED)) {
    const auto lneg = is_neg_signed();
    const auto rneg = rhs.is_neg_signed();
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
inline void BitsBase<T, BT, ST>::read_real(std::istream& is) {
  double d;
  is >> d;
  val_.resize(64/bits_per_word());
  *reinterpret_cast<double*>(val_.data()) = d;
  size_ = 64;
  type_ = Type::REAL;
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::read_2_8_16(std::istream& is, size_t base) {
  // Input Buffer:
  std::string s;
  is >> s;

  // Reset to a 1-bit unsigned value
  shrink_to_bool(false);
  type_ = Type::UNSIGNED;

  // Return immediately if reading s failed.
  if (s.empty()) {
    return;
  }

  // How many bits do we generate per character? Resize internal storage.
  const auto step = (base == 2) ? 1 : (base == 8) ? 3 : 4;
  extend_to(step * s.length());

  // Walk over the buffer from lowest to highest order (that's in reverse)
  size_t word = 0;
  size_t idx = 0;
  for (int i = s.length()-1; i >= 0; --i) {
    // Convert character to bits
    const T bits = static_cast<bool>(isalpha(s[i])) ? ((s[i]-'a')+10) : (s[i]-'0');
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
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::read_10(std::istream& is) {
  // Check for negative
  const auto is_neg = is.peek() == '-';
  if (is_neg) {
    is.get();
  }

  // Input Buffer:
  std::string s;
  is >> s;

  // Reset to 1-bit signed value
  shrink_to_bool(false);
  type_ = is_neg ? Type::SIGNED : Type::UNSIGNED;

  // Halve decimal string until it becomes zero. Add 1s when least significant
  // digit is odd, 0s otherwise. An empty string will leave this loop
  // immediately.
  for (size_t i = 1; !dec_zero(s); ++i) {
    extend_to(i);
    set(i-1, (s.back()-'0') % 2); 
    dec_halve(s);
  }
  // Add padding for sign bit and negate if necessary
  extend_to(size_+1);
  if (is_neg) {
    invert_add_one();
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::write_real(std::ostream& os) const {
  os << to_double();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::write_2_8_16(std::ostream& os, size_t base) const {
  // Cast to unsigned
  if (type_ != Type::UNSIGNED) {
    auto temp = *this;
    temp.cast_type(Type::UNSIGNED);
    return temp.write_2_8_16(os, base);
  }

  // TODO(eschkufz): This would be a lot faster if we traversed the value from
  // most to least significant bit and didn't store a temporary array.

  // Output Buffer:
  std::vector<uint8_t> buf;

  // How many bits do we consume per character? Make a mask.
  const size_t step = (base == 2) ? 1 : (base == 8) ? 3 : 4;
  const auto mask = (static_cast<T>(1) << step) - 1;

  // Walk over the string from lowest to highest order
  size_t off = 0;
  for (size_t i = 0, ie = size(); i < ie; i += step) {
    const auto pos = i / bits_per_word();
    // Extract mask bits 
    buf.push_back((val_[pos] >> off) & mask);
    off += step;
    // Easy case: Step divides words evenly 
    if (off == bits_per_word()) {
      off = 0;
    } 
    // Hard case: Look ahead for more bits
    else if (off > bits_per_word()) {
      off %= bits_per_word();
      if ((pos+1) != val_.size()) {
        buf.back() |= (val_[pos+1] & ((static_cast<T>(1) << off) - 1)) << (step - off);
      }
    }
  }

  // Print the result
  for (int i = buf.size()-1; i >= 0; --i) {
    if (buf[i] > 9) {
      os << static_cast<char>('a' + (buf[i] - 10));
    } else {
      os << static_cast<int>(buf[i]);
    }
  }
}


template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::write_10(std::ostream& os) const {
  // Cast reals down to integers
  if (type_ == Type::REAL) {
    auto temp = *this;
    temp.cast_type(Type::SIGNED);
    return temp.write_10(os); 
  } 

  // Check whether this is a negative number
  const auto is_neg = is_neg_signed();
   
  // Print negative and invert if necessary
  if (is_neg) {
    os << "-";
    const_cast<BitsBase*>(this)->invert_add_one();
  }
  // Output Buffer (lsb in index 0):
  std::string buf = "0";
  // Walk binary string from highest to lowest.
  // Double result each time, add 1s as they appear.
  for (int i = size_-1; i >= 0; --i) {
    dec_double(buf);
    if (get(i)) {
      dec_inc(buf);
    }
  }
  // Print the result in reverse
  for (int i = buf.length()-1; i >= 0; --i) {
    os << buf[i];
  }
  // Restore bits if they were inverted
  if (is_neg) {
    const_cast<BitsBase*>(this)->invert_add_one();
  }
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::dec_halve(std::string& s) const {
  auto next_carry = 0;
  for (size_t i = 0, ie = s.length(); i < ie; ++i) {
    const auto val = s[i] - '0';
    auto carry = next_carry;
    next_carry = ((val % 2) != 0) ? 5 : 0;
    s[i] = ((val/2) + carry) + '0';
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
    val = (2*val) + carry;
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
inline void BitsBase<T, BT, ST>::bitwise_sll_const(const BitsBase& lhs, size_t samt) {
  assert(!is_real() && !lhs.is_real());
  assert(size() == lhs.size());
  
  // Easy Case: We're not actually shifting
  if (samt == 0) {
    for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
      val_[i] = lhs.val_[i];
    }
    return;
  }

  // This algorithm works from highest to lowest order, one word at a time
  // word: The current word we're looking at
  // top/bottom: The words that will be shifted into word; word can equal top

  // How many words ahead is bottom?
  const auto delta = ((samt + bits_per_word()) - 1) / bits_per_word();
  // How many bits are we taking from bottom and shifting top?
  const auto bamt = samt % bits_per_word();
  // Create a mask for extracting the highest bamt bits from bottom
  const auto mamt = bits_per_word() - bamt;
  const auto mask = ((static_cast<T>(1) << bamt) - 1) << mamt;

  // Work our way down until bottom hits zero
  int w = val_.size() - 1;
  for (int b = w-delta; b >= 0; --w, --b) {
    if (bamt == 0) {
      val_[w] = lhs.val_[b];
    } else {
      val_[w] = (lhs.val_[b+1] << bamt) | ((lhs.val_[b] & mask) >> mamt);
    }
  }
  // There's one more block to build where bottom is implicitly zero
  val_[w--] = (bamt == 0) ? static_cast<T>(0) : (lhs.val_[0] << bamt);
  // Everything else is zero
  for (; w >= 0; --w) {
    val_[w] = static_cast<T>(0);
  }
  // Trim the top and we're done
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::bitwise_sxr_const(const BitsBase& lhs, size_t samt, bool arith) {
  assert(!is_real() && !lhs.is_real());
  assert(size() == lhs.size());

  // Easy Case: We're not actually shifting
  if (samt == 0) {
    for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
      val_[i] = lhs.val_[i];
    }
    return;
  }

  // This algorithm works from lowest to highest order, one word at a time
  // word: The current word we're looking at
  // top/bottom: The words that will be shifted into word; word can equal bottom

  // Is the highest order bit a 1 and do we care?
  const auto idx = (size_-1) % bits_per_word();
  const auto hob = arith && ((lhs.val_.back() & (static_cast<T>(1) << idx)) != 0); 
  // How many words ahead is top?
  const auto delta = ((samt + bits_per_word()) - 1) / bits_per_word();
  // How many bits are we taking from top and shifting bottom?
  const auto bamt = samt % bits_per_word();
  // Create a mask for extracting the lowest bamt bits from top
  const auto mamt = bits_per_word() - bamt;
  const auto mask = (static_cast<T>(1) << bamt) - 1;
  // val_ is an array of unsigned values. If we are working with signed values,
  // we want the top bits of the top-most word to be filled with ones.
  // This is only used in the case where hob is set and the top word is not
  // completely full.
  const auto upper_most_word = ((static_cast<T>(-1) << idx) | lhs.val_.back());

  // Work our way up until top goes out of range
  size_t w = 0;
  for (size_t t = w+delta, te = val_.size(); t < te; ++w, ++t) {
    if (bamt == 0) {
      val_[w] = lhs.val_[t];
    } else {
      const auto upper_most = (t == (val_.size() - 1)) ? upper_most_word : lhs.val_[t];
      val_[w] = (lhs.val_[t-1] >> bamt) | ((upper_most & mask) << mamt);
    }
  }
  // There's one more block to build where top is implicitly zero
  if (hob) {
    val_[w++] = (bamt == 0) ? static_cast<T>(-1) : ((upper_most_word >> bamt) | (mask << mamt));
  } else {
    val_[w++] = (bamt == 0) ? static_cast<T>(0) : (lhs.val_.back() >> bamt);
  }
  // Everything else is zero or padded 1s
  for (size_t we = val_.size(); w < we; ++w) {
    val_[w] = hob ? static_cast<T>(-1) : static_cast<T>(0);
  }
  // Trim since we could have introduced trailing 1s
  trim();
}

template <typename T, typename BT, typename ST>
inline T BitsBase<T, BT, ST>::signed_get(size_t n) const {
  // Difficult case: Do we need to sign extend this value?
  if (n == (val_.size()-1)) {
    const auto top = size_ % bits_per_word();
    if (!is_neg_signed() || (top == 0)) {
      return val_[n];
    }
    return val_[n] | (static_cast<T>(-1) << top);
  }
  // Easier Case: Do we need to return all 1s or 0s?
  else if (n >= val_.size()) {
    return is_neg_signed() ? static_cast<T>(-1) : static_cast<T>(0);
  }
  // Easiest Case: Just return what's there
  else {
    return val_[n]; 
  }
}

template <typename T, typename BT, typename ST>
inline bool BitsBase<T, BT, ST>::is_neg_signed() const {
  return (type_ == Type::SIGNED) && get(size_-1);
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::trim() {
  // How many bits do we care about in the top word?
  const auto trailing = size_ % bits_per_word();
  // Zero means we're full
  if (trailing == 0) {
    return;
  }
  // Otherwise, mask these off
  const auto mask = (static_cast<T>(1) << trailing) - 1;
  val_.back() &= mask;
}
    
template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::extend_to(size_t n) {
  assert(n >= size_);
  const auto words = ((n + bits_per_word()) - 1) / bits_per_word();
  val_.resize(words, static_cast<T>(0));
  size_ = n;
}
   
template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::sign_extend_to(size_t n) {
  assert(n >= size_);
  const auto words = ((n + bits_per_word()) - 1) / bits_per_word();
  if (is_neg_signed()) {
    val_.back() |= (static_cast<T>(-1) << (size_ % bits_per_word()));
    val_.resize(words, static_cast<T>(-1));
    size_ = n;
    trim();
  } else {
    val_.resize(words, static_cast<T>(0));
    size_ = n;
  }
}
   
template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::shrink_to(size_t n) {
  assert(n <= size_);
  const auto words = ((n + bits_per_word()) - 1) / bits_per_word();
  val_.resize(words, static_cast<T>(0));
  size_ = n;
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::shrink_to_bool(bool b) {
  val_.resize(1, static_cast<T>(0));
  val_[0] = b ? 1 : 0;
  size_ = 1;
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::invert_add_one() {
  T carry = 1;
  for (size_t i = 0, ie = val_.size(); i < ie; ++i) {
    val_[i] = ~val_[i];
    const T sum = val_[i] + carry;
    carry = (sum < val_[i]) ? static_cast<T>(1) : static_cast<T>(0);
    val_[i] = sum;
  }
  trim();
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::cast_int_to_real() {
  assert(type_ != Type::REAL);
  const auto d = to_double();
  val_.resize(64/bits_per_word());
  *reinterpret_cast<double*>(val_.data()) = d;
  size_ = 64;
  type_ = Type::REAL;
}

template <typename T, typename BT, typename ST>
inline void BitsBase<T, BT, ST>::cast_real_to_int(bool s) {
  assert(type_ == Type::REAL);
  auto d = std::round(*reinterpret_cast<const double*>(val_.data()));
  const auto is_neg = d < 0.0;
  if (is_neg) {
    d = -d;
  }   

  shrink_to_bool(false);
  for (size_t i = 1; d > 0; d = std::floor(d/2), ++i) {
    extend_to(i);
    set(i-1, static_cast<T>(d) % 2);
  }
  if (s) {
    extend_to(size_+1);
  }
  if (is_neg) {
    invert_add_one();
  }
  type_ = s ? Type::SIGNED : Type::UNSIGNED;
}

template <typename T, typename BT, typename ST>
inline constexpr size_t BitsBase<T, BT, ST>::bits_per_word() const {
  return 8 * bytes_per_word();
}

template <typename T, typename BT, typename ST>
inline constexpr size_t BitsBase<T, BT, ST>::bytes_per_word() const {
  return sizeof(T);
}

template <typename T, typename BT, typename ST>
inline constexpr double BitsBase<T, BT, ST>::range() const {
  return std::pow(2, bits_per_word());
}

} // namespace cascade

#endif
