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

#ifndef CASCADE_SRC_TARGET_CORE_COMMON_SCANF_H
#define CASCADE_SRC_TARGET_CORE_COMMON_SCANF_H

#include <cctype>
#include <iostream>
#include "base/bits/bits.h"
#include "verilog/analyze/evaluate.h"
#include "verilog/ast/ast.h"

namespace cascade {

struct Scanf {
  void read(std::istream& is, Evaluate* eval, const GetStatement* gs);
};

inline void Scanf::read(std::istream& is, Evaluate* eval, const GetStatement* gs) {
  const auto format = gs->get_fmt()->get_readable_val();
  if (format[0] != '%') {
    for (auto c : format) {
      if (isspace(c)) {
        while (isspace(is.peek())) {
          is.get();
        }
      } else if (c != is.get()) {
        break;
      }
    }
    return;
  }

  assert(gs->is_non_null_var());
  Bits val;
  switch (format[1]) {
    case '_': 
      if (eval->get_type(gs->get_var()) == Bits::Type::REAL) {
        val.read(is, 1);
      } else {
        val.read(is, 10);
      }
      break;
    case 'b':
    case 'B':
      val.read(is, 2);
      break;
    case 'c':
    case 'C': 
      val = Bits(static_cast<char>(is.get()));
      break;
    case 'd':
    case 'D': 
      val.read(is, 10);
      break;
    case 'e':
    case 'E':
    case 'f':
    case 'F':
    case 'g':
    case 'G': 
      val.read(is, 1);
      break;
    case 'h':
    case 'H': 
      val.read(is, 16);
      break;
    case 'o':
    case 'O': 
      val.read(is, 8);
      break;
    case 's':
    case 'S': {
      std::string s;
      is >> s;
      val = Bits(s);
      break;
    }
    case 'u':
    case 'U': 
      val.read(is, 16);
      break;
    default: 
      assert(false);
      break;
  }
  eval->assign_value(gs->get_var(), val);
}

} // namespace cascade

#endif

