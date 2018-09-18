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
#include <cmath>
#include <cstdio>
#include <gmp.h>
#include <iostream>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include "src/base/serial/serializable.h"

#ifdef __APPLE__
#include "ext/memorymapping/src/fmemopen.h"
#endif

namespace cascade {

// This class is the fundamental representation of a bit string. It imposes the
// semantics of Verilog expressions onto the GNU Multiple Precision Numeric
// Library.

class Bits : public Serializable {
  public:
    // Constructors:
    Bits();
    Bits(bool b);
    Bits(size_t n, int32_t val);
    Bits(size_t n, uint32_t val);
    Bits(const Bits& rhs);
    Bits(Bits&& rhs);
    Bits& operator=(const Bits& rhs);
    Bits& operator=(Bits&& rhs);
    ~Bits() override;
    
    // Serial I/O
    void read(std::istream& is, size_t base);
    void write(std::ostream& os, size_t base) const;
    size_t deserialize(std::istream& is) override;
    size_t serialize(std::ostream& os) const override;

    // Casts:
    bool to_bool() const;
    int32_t to_int() const;
    uint32_t to_uint() const;

    // Size:
    size_t size() const;
    void resize(size_t n);

    // Bitwise Operators:
    Bits& bitwise_and(const Bits& rhs);
    Bits& bitwise_or(const Bits& rhs);
    Bits& bitwise_xor(const Bits& rhs);
    Bits& bitwise_xnor(const Bits& rhs);
    Bits& bitwise_sll(const Bits& rhs);
    Bits& bitwise_sal(const Bits& rhs);
    Bits& bitwise_slr(const Bits& rhs);
    Bits& bitwise_sar(const Bits& rhs);
    Bits& bitwise_not();

    // Arithmetic Operators:
    Bits& arithmetic_plus();
    Bits& arithmetic_plus(const Bits& rhs);
    Bits& arithmetic_minus();
    Bits& arithmetic_minus(const Bits& rhs);
    Bits& arithmetic_multiply(const Bits& rhs);
    Bits& arithmetic_divide(const Bits& rhs);
    Bits& arithmetic_mod(const Bits& rhs);
    Bits& arithmetic_pow(const Bits& rhs);

    // Logical Operators:
    Bits& logical_and(const Bits& rhs);
    Bits& logical_or(const Bits& rhs);
    Bits& logical_not();
    Bits& logical_eq(const Bits& rhs);
    Bits& logical_ne(const Bits& rhs);
    Bits& logical_lt(const Bits& rhs);
    Bits& logical_lte(const Bits& rhs);
    Bits& logical_gt(const Bits& rhs);
    Bits& logical_gte(const Bits& rhs);

    // Reduction Operators:
    Bits& reduce_and();
    Bits& reduce_nand();
    Bits& reduce_or();
    Bits& reduce_nor();
    Bits& reduce_xor();
    Bits& reduce_xnor();

    // Concatenation Operators:
    Bits& concat(const Bits& rhs);

    // Slicing Operators:
    Bits& slice(size_t idx);
    Bits& slice(size_t msb, size_t lsb);

    // Comparison Operators:
    bool eq(const Bits& rhs, size_t idx);
    bool eq(const Bits& rhs, size_t msb, size_t lsb);

    // Bitwise Operators:
    Bits& flip(size_t idx);
    Bits& set(size_t idx, bool b);
    template <typename T>
    T read_word(size_t n);
    template <typename T>
    void write_word(size_t n, T t);

    // Assignment Operators:
    Bits& assign(const Bits& rhs);
    Bits& assign(size_t idx, const Bits& rhs);
    Bits& assign(size_t msb, size_t lsb, const Bits& rhs);

    // Built-in Operators:
    bool operator==(const Bits& rhs) const;
    bool operator!=(const Bits& rhs) const;
    bool operator<(const Bits& rhs) const;

    // Swap Operator:
    void swap(Bits& rhs);

  private:
    // Bit state:
    //
    // INVARIANT: The bit-width of val_ is always equal to size_.  libgmp will
    // automatically size extend (sometimes unexpectedly).  The implementation
    // of this class must account for this (see below).
    mpz_t val_;
    uint16_t size_;

    // Scratch space 
    //
    // Several of the methods in this class require the use of bit mask.
    // Allocating permanent scratch space for these masks rather than
    // dynamically allocating them on demand seems to be more performant. That
    // said, if we could eliminate these buffers, we could significantly
    // improve our memory footprint.
    mpz_t scratch1_;
    mpz_t scratch2_;

    // Helper methods
    //
    // Enforces the invariant that val_ has the same bit-width as size_. This
    // method should be called whenever size extension may be necessary (say
    // for bitwise or where rhs is larger than lhs).
    void extend(const Bits& rhs);
    // Enforces the invariant that val_ has the same bit-width as size_.  This
    // method should be called whenever there is some chance that libgmp has
    // size extended val_ beyond size_ (say for arithmetic addition) where 2'2
    // + 2'2 yields 3'4, when it should yield 2'0.
    void trim();
};

inline Bits::Bits() : Bits(false) { }

inline Bits::Bits(bool b) : Bits(1, b ? (uint32_t)1 : (uint32_t)0) { }

inline Bits::Bits(size_t n, int32_t val) { 
  assert(n > 0);
  mpz_init2(val_, n);
  mpz_set_si(val_, val);
  size_ = n;
  assert(mpz_sizeinbase(val_, 2) <= size_);

  mpz_init(scratch1_);
  mpz_init(scratch2_);
}

inline Bits::Bits(size_t n, uint32_t val) { 
  assert(n > 0);
  mpz_init2(val_, n);
  mpz_set_ui(val_, val);
  size_ = n;
  assert(mpz_sizeinbase(val_, 2) <= size_);

  mpz_init(scratch1_);
  mpz_init(scratch2_);
}

inline Bits::Bits(const Bits& rhs) {
  mpz_init_set(val_, rhs.val_);
  size_ = rhs.size_;

  mpz_init(scratch1_);
  mpz_init(scratch2_);
}

inline Bits::Bits(Bits&& rhs) : Bits{} {
  swap(rhs);
}

inline Bits& Bits::operator=(const Bits& rhs) {
  mpz_set(val_, rhs.val_);
  size_ = rhs.size_;
  return *this;
}

inline Bits& Bits::operator=(Bits&& rhs) {
  swap(rhs);
  return *this;
}

inline Bits::~Bits() {
  mpz_clear(val_);
  mpz_clear(scratch1_);
  mpz_clear(scratch2_);
}

inline void Bits::read(std::istream& is, size_t base) {
  // TODO: Is there a nicer way to do this that doesn't use fmemopen?
  std::string s;
  is >> s;  

  auto f = fmemopen((void*)s.c_str(), s.length(), "r");
  mpz_inp_str(val_, f, base);
  size_ = mpz_sizeinbase(val_, 2);
  fclose(f);
}

inline void Bits::write(std::ostream& os, size_t base) const {
  // TODO: Is there a nicer way to do this that doesn't use fmemopen?
  char buffer[1024];
  auto f = fmemopen(buffer, 1024, "w");
  const auto s = mpz_out_str(f, base, val_);
  fclose(f);

  os << std::string(buffer, s);
}

inline size_t Bits::deserialize(std::istream& is) {
  // TODO: This method is ignoring sign
  char buffer[1024];
  uint16_t l = 0;

  is.read((char*)&size_, sizeof(size_));
  is.read((char*)&l, sizeof(l));
  is.read((char*)buffer, l);

  size_t len = l;
  mpz_import(val_, len, 1, sizeof(char), 0, 0, buffer);

  return sizeof(size_) + sizeof(l) + len;
}

inline size_t Bits::serialize(std::ostream& os) const {
  // TODO: This method is ignoring sign
  char buffer[1024];
  size_t len;

  mpz_export(buffer, &len, 1, sizeof(char), 0, 0, val_);
  uint16_t l = len;

  os.write((char*)&size_, sizeof(size_));
  os.write((char*)&l, sizeof(l));
  os.write((char*)buffer, len);

  return sizeof(size_) + sizeof(l) + len;
}

inline bool Bits::to_bool() const {
  return mpz_cmp_ui(val_, 0) != 0;
}

inline int32_t Bits::to_int() const {
  assert(size() <= 32);
  return mpz_get_si(val_);
}

inline uint32_t Bits::to_uint() const {
  assert(size() <= 32);
  return mpz_get_ui(val_);
}

inline size_t Bits::size() const {
  return size_;
}

inline void Bits::resize(size_t n) {
  if (n < size()) {
    size_ = n;
    trim();
  } else {
    size_ = n;
  }
}

inline Bits& Bits::bitwise_and(const Bits& rhs) {
  extend(rhs);
  mpz_and(val_, val_, rhs.val_);
  return *this;
}

inline Bits& Bits::bitwise_or(const Bits& rhs) {
  extend(rhs);
  mpz_ior(val_, val_, rhs.val_);
  return *this;
}

inline Bits& Bits::bitwise_xor(const Bits& rhs) {
  extend(rhs);
  mpz_xor(val_, val_, rhs.val_);
  return *this;
}

inline Bits& Bits::bitwise_xnor(const Bits& rhs) {
  extend(rhs);
  mpz_xor(val_, val_, rhs.val_);
  mpz_com(val_, val_);
  return *this;
}

inline Bits& Bits::bitwise_sll(const Bits& rhs) {
  mpz_mul_2exp(val_, val_, rhs.to_int());
  trim();
  return *this;
}

inline Bits& Bits::bitwise_sal(const Bits& rhs) {
  return bitwise_sll(rhs);
}

inline Bits& Bits::bitwise_slr(const Bits& rhs) {
  mpz_tdiv_q_2exp(val_, val_, rhs.to_int());
  return *this;
}

inline Bits& Bits::bitwise_sar(const Bits& rhs) {
  mpz_fdiv_q_2exp(val_, val_, rhs.to_int());
  return *this;
}

inline Bits& Bits::bitwise_not() {
  mpz_com(val_, val_);
  return *this;
}

inline Bits& Bits::arithmetic_plus() {
  // Does nothing.
  return *this;
}

inline Bits& Bits::arithmetic_plus(const Bits& rhs) {
  mpz_add(val_, val_, rhs.val_);
  resize(std::max(size(), rhs.size()));
  return *this;
}

inline Bits& Bits::arithmetic_minus() {
  mpz_neg(val_, val_);
  return *this;
}

inline Bits& Bits::arithmetic_minus(const Bits& rhs) {
  mpz_sub(val_, val_, rhs.val_);
  resize(std::max(size(), rhs.size()));
  return *this;
}

inline Bits& Bits::arithmetic_multiply(const Bits& rhs) {
  mpz_mul(val_, val_, rhs.val_);
  resize(std::max(size(), rhs.size()));
  return *this;
}

inline Bits& Bits::arithmetic_divide(const Bits& rhs) {
  mpz_tdiv_q(val_, val_, rhs.val_);
  resize(std::max(size(), rhs.size()));
  return *this;
}

inline Bits& Bits::arithmetic_mod(const Bits& rhs) {
  mpz_mod(val_, val_, rhs.val_);
  resize(std::max(size(), rhs.size()));
  return *this;
}

inline Bits& Bits::arithmetic_pow(const Bits& rhs) {
  mpz_pow_ui(val_, val_, rhs.to_int());
  trim();
  return *this;
}

inline Bits& Bits::logical_and(const Bits& rhs) {
  mpz_set_ui(val_, to_bool() && rhs.to_bool() ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::logical_or(const Bits& rhs) {
  mpz_set_ui(val_, to_bool() || rhs.to_bool() ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::logical_not() {
  mpz_set_ui(val_, mpz_cmp_ui(val_, 0) == 0 ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::logical_eq(const Bits& rhs) {
  mpz_set_ui(val_, mpz_cmp(val_, rhs.val_) == 0 ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::logical_ne(const Bits& rhs) {
  mpz_set_ui(val_, mpz_cmp(val_, rhs.val_) != 0 ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::logical_lt(const Bits& rhs) {
  mpz_set_ui(val_, mpz_cmp(val_, rhs.val_) < 0 ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::logical_lte(const Bits& rhs) {
  mpz_set_ui(val_, mpz_cmp(val_, rhs.val_) <= 0 ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::logical_gt(const Bits& rhs) {
  mpz_set_ui(val_, mpz_cmp(val_, rhs.val_) > 0 ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::logical_gte(const Bits& rhs) {
  mpz_set_ui(val_, mpz_cmp(val_, rhs.val_) >= 0 ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::reduce_and() {
  mpz_set_ui(val_, mpz_popcount(val_) == size_ ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::reduce_nand() {
  mpz_set_ui(val_, mpz_popcount(val_) == size_ ? 0 : 1);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::reduce_or() {
  mpz_set_ui(val_, mpz_cmp_ui(val_, 0) != 0 ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::reduce_nor() {
  mpz_set_ui(val_, mpz_cmp_ui(val_, 0) == 0 ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::reduce_xor() {
  mpz_set_ui(val_, mpz_odd_p(val_) ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::reduce_xnor() {
  mpz_set_ui(val_, mpz_even_p(val_) ? 1 : 0);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::concat(const Bits& rhs) {
  mpz_mul_2exp(val_, val_, rhs.size());
  mpz_ior(val_, val_, rhs.val_);
  resize(size() + rhs.size());
  return *this;
}

inline Bits& Bits::slice(size_t idx) {
  assert(idx < size());
  mpz_tdiv_q_2exp(val_, val_, idx);
  size_ = 1;
  trim();
  return *this;
}

inline Bits& Bits::slice(size_t msb, size_t lsb) {
  assert(msb >= lsb);
  assert(msb < size());
  mpz_tdiv_q_2exp(val_, val_, lsb);
  resize(msb-lsb+1);
  return *this;
}

inline bool Bits::eq(const Bits& rhs, size_t idx) {
  return mpz_tstbit(val_, idx) == mpz_tstbit(rhs.val_, 0);
}

inline bool Bits::eq(const Bits& rhs, size_t msb, size_t lsb) {
  assert(msb >= lsb);
  assert(msb < size());

  // Copy val_ and slice
  mpz_set(scratch1_, val_);
  mpz_tdiv_q_2exp(scratch1_, scratch1_, lsb);
  mpz_set_ui(scratch2_, 1);
  mpz_mul_2exp(scratch2_, scratch2_, msb-lsb+1);
  mpz_sub_ui(scratch2_, scratch2_, 1);
  mpz_and(scratch1_, scratch1_, scratch2_);

  return mpz_cmp(scratch1_, rhs.val_) == 0;
}

inline Bits& Bits::flip(size_t idx) {
  assert(idx < size());
  mpz_combit(val_, idx);
  return *this;
}

inline Bits& Bits::set(size_t idx, bool b) {
  assert(idx < size());
  if (b) {
    mpz_setbit(val_, idx);
  } else {
    mpz_clrbit(val_, idx);
  }
  return *this;
}

template <typename T>
inline T Bits::read_word(size_t n) {
  const auto msb = std::min(size(), 8*sizeof(T)*(n+1));
  const auto lsb = 8*sizeof(T)*n;

  mpz_set(scratch1_, val_);
  mpz_tdiv_q_2exp(scratch1_, scratch1_, lsb);
  mpz_set_ui(scratch2_, 1);
  mpz_mul_2exp(scratch2_, scratch2_, msb-lsb+1);
  mpz_sub_ui(scratch2_, scratch2_, 1);
  mpz_and(scratch1_, scratch1_, scratch2_);

  return mpz_get_ui(scratch1_);
}

template <typename T>
inline void Bits::write_word(size_t n, T t) {
  const auto msb = std::min(size(), 8*sizeof(T)*(n+1));
  const auto lsb = 8*sizeof(T)*n;

  mpz_set_ui(scratch1_, 1);
  mpz_mul_2exp(scratch1_, scratch1_, msb-lsb+1);
  mpz_sub_ui(scratch1_, scratch1_, 1);
  mpz_mul_2exp(scratch1_, scratch1_, lsb);
  mpz_com(scratch1_, scratch1_);

  mpz_and(val_, val_, scratch1_);

  mpz_set_ui(scratch1_, t);
  mpz_mul_2exp(scratch1_, scratch1_, lsb);
  mpz_ior(val_, val_, scratch1_);
}

inline Bits& Bits::assign(const Bits& rhs) {
  mpz_set(val_, rhs.val_);
  if (rhs.size() > size()) {
    trim();
  }
  return *this;
}

inline Bits& Bits::assign(size_t idx, const Bits& rhs) {
  assert(idx < size());
  if (mpz_tstbit(rhs.val_, 0)) {
    mpz_setbit(val_, idx);
  } else {
    mpz_clrbit(val_, idx);
  }
  return *this;
}

inline Bits& Bits::assign(size_t msb, size_t lsb, const Bits& rhs) {
  if (msb == lsb) {
    return assign(msb, rhs);
  }

  assert(msb >= lsb);
  assert(msb < size());

  mpz_set_ui(scratch1_, 1);
  mpz_mul_2exp(scratch1_, scratch1_, msb-lsb+1);
  mpz_sub_ui(scratch1_, scratch1_, 1);

  mpz_set(scratch2_, scratch1_);
  mpz_mul_2exp(scratch2_, scratch2_, lsb);
  mpz_com(scratch2_, scratch2_);

  mpz_and(val_, val_, scratch2_);
  mpz_and(scratch1_, scratch1_, rhs.val_);
  mpz_mul_2exp(scratch1_, scratch1_, lsb);
  mpz_ior(val_, val_, scratch1_);

  return *this;
}

inline bool Bits::operator==(const Bits& rhs) const {
  return size_ == rhs.size_ && mpz_cmp(val_, rhs.val_) == 0;
}

inline bool Bits::operator!=(const Bits& rhs) const {
  return !(*this == rhs);
}

inline bool Bits::operator<(const Bits& rhs) const {
  return size_ < rhs.size_ || mpz_cmp(val_, rhs.val_) < 0;
}

inline void Bits::swap(Bits& rhs) {
  std::swap(val_, rhs.val_);
  std::swap(size_, rhs.size_);
}

inline void Bits::extend(const Bits& rhs) {
  if (rhs.size_ > size_) {
    mpz_realloc2(val_, rhs.size_);
    size_ = rhs.size_;
  }
}

inline void Bits::trim() {
  mpz_set_ui(scratch1_, 1);
  mpz_mul_2exp(scratch1_, scratch1_, size_);
  mpz_sub_ui(scratch1_, scratch1_, 1);
  mpz_and(val_, val_, scratch1_);
}

} // namespace cascade

#endif
