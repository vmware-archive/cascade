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

#ifndef CASCADE_SRC_TARGET_CORE_COMMON_PRINTF_H
#define CASCADE_SRC_TARGET_CORE_COMMON_PRINTF_H

#include <cstdio>
#include <iostream>
#include "verilog/analyze/evaluate.h"
#include "verilog/ast/ast.h"

namespace cascade {

struct Printf {
  void write(std::ostream& os, Evaluate* eval, const PutStatement* ps);
};

inline void Printf::write(std::ostream& os, Evaluate* eval, const PutStatement* ps) {
  const auto format = ps->get_fmt()->get_readable_val();
  if (format[0] != '%') {
    os << format;
    return;
  }

  assert(ps->is_non_null_expr());
  const auto* expr = ps->get_expr();

  switch (format[1]) {
    case '_':
      if (eval->get_type(expr) == Bits::Type::REAL) {
        eval->get_value(expr).write(os, 1);
      } else {
        eval->get_value(expr).write(os, 10);
      }
      break;
    case 'b':
    case 'B': 
      eval->get_value(expr).write(os, 2);
      return;
    case 'c':
    case 'C':
      os << (eval->get_value(expr).to_char());
      return;
    case 'd':
    case 'D':
      eval->get_value(expr).write(os, 10);
      return;
    case 'h':
    case 'H': 
      eval->get_value(expr).write(os, 16);
      return;
    case 'o':
    case 'O': 
      eval->get_value(expr).write(os, 8);
      return;
    case 's':
    case 'S': 
      os << eval->get_value(expr).to_string();
      return;
    case 'u':
    case 'U':
      eval->get_value(expr).write(os, 16);
      return;
    default: 
      break;
  } 
 
  char buffer[1024]; 
  std::snprintf(buffer, 1024, format.c_str(), eval->get_value(expr).to_double());
  os << buffer;
}

} // namespace cascade

#endif
