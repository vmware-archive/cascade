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
    enum Format {
      UNSIGNED = 0,
      DEC,
      BIN,
      OCT,
      HEX
    };
  
    // Constructors:
    Number(const std::string& val, Format format_ = UNSIGNED, size_t size = 0);
    Number(const Bits& val, Format format = UNSIGNED, size_t size = 0);
    ~Number() override = default;

    // Node Interface:
    NODE(Number, LEAF(val), LEAF(format))
    // Get/Set:
    LEAF_GET_SET(val)
    LEAF_GET_SET(format)

  private:
    LEAF_ATTR(Bits, val);
    LEAF_ATTR(Format, format);
};

inline Number::Number(const std::string& val, Format format, size_t size) : Primary() {
  parent_ = nullptr;
  std::stringstream ss(val);
  switch (format) {
    case UNSIGNED:
    case DEC:
      val_.read(ss, 10);  
      break;
    case BIN:
      val_.read(ss, 2);
      break;
    case OCT:
      val_.read(ss, 8);
      break;
    case HEX:
      val_.read(ss, 16);
      break;
    default:
      assert(false);
      break;
  }
  if (size > 0) {
    val_.resize(size);
  }
  format_ = format;
}

inline Number::Number(const Bits& val, Format format, size_t size) : Primary() {
  parent_ = nullptr;
  val_ = val;
  if (size > 0) {
    val_.resize(size);
  }
  format_ = format;
}

} // namespace cascade

#endif
