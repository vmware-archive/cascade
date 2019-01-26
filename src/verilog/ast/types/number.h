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

#ifndef CASCADE_SRC_VERILOG_AST_NUMBER_H
#define CASCADE_SRC_VERILOG_AST_NUMBER_H

#include <cassert>
#include <sstream>
#include <string>
#include "src/verilog/ast/types/macro.h"
#include "src/verilog/ast/types/primary.h"

namespace cascade {

class Number : public Primary {
  public:
    // Supporting Concepts:
    enum class Format : uint8_t {
      UNBASED = 0,
      DEC,
      BIN,
      OCT,
      HEX
    };
  
    // Constructors:
    Number(const std::string& val, Format format_ = Format::UNBASED, size_t size = 0, bool is_signed = false);
    Number(const Bits& val, Format format = Format::UNBASED);
    ~Number() override = default;

    // Node Interface 
    NODE(Number)
    Number* clone() const override;

    // Stripped down get/set:
    const Bits& get_val() const;
    void set_val(const Bits& val);
    Format get_format() const;
    void set_format(Format format);

    // NOTE: All attributes and decorations are stored in Node::common_ and
    // Expression::bit_val_
};

inline Number::Number(const std::string& val, Format format, size_t size, bool is_signed) : Primary() {
  parent_ = nullptr;

  bit_val_.resize(1);
  std::stringstream ss(val);
  switch (format) {
    case Format::UNBASED:
    case Format::DEC:
      bit_val_[0].read(ss, 10);  
      break;
    case Format::BIN:
      bit_val_[0].read(ss, 2);
      break;
    case Format::OCT:
      bit_val_[0].read(ss, 8);
      break;
    case Format::HEX:
      bit_val_[0].read(ss, 16);
      break;
    default:
      assert(false);
      break;
  }
  if (size > 0) {
    bit_val_[0].resize(size);
  }
  bit_val_[0].set_signed(is_signed);

  set_format(format);
  set_flag<5>(bit_val_[0].is_signed());
  Node::set_val<6,26>(bit_val_[0].size());
}

inline Number::Number(const Bits& val, Format format) : Primary() {
  parent_ = nullptr;

  bit_val_.push_back(val);
  set_format(format);
  set_flag<5>(bit_val_[0].is_signed());
  Node::set_val<6,26>(bit_val_[0].size());
}

inline Number* Number::clone() const {
  return new Number(get_val(), get_format());
}

inline const Bits& Number::get_val() const {
  return bit_val_[0];
}

inline void Number::set_val(const Bits& val) {
  bit_val_[0] = val;
}

inline Number::Format Number::get_format() const {
  return static_cast<Format>(Node::get_val<2,3>());  
}

inline void Number::set_format(Format f) {
  Node::set_val<2,3>(static_cast<uint32_t>(f));
}

} // namespace cascade

#endif
