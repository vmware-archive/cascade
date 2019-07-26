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
  void write(std::ostream& os, Evaluate* eval, const PutStatement* ps) const;
};

inline void Printf::write(std::ostream& os, Evaluate* eval, const PutStatement* ps) const {
  const auto* fmt = ps->get_fmt()->get_readable_val().c_str();
  if (fmt[0] != '%') {
    os << fmt;
    return;
  }

  assert(ps->is_non_null_expr());
  const auto& val = eval->get_value(ps->get_expr());

  switch (fmt[1]) {
    case '_':
      val.write(os, 0);
      return;
    case 'b':
    case 'B': 
      val.write(os, 2);
      return;
    case 'c':
    case 'C':
      os << val.to_char();
      return;
    case 'd':
    case 'D':
      val.write(os, 10);
      return;
    case 'h':
    case 'H': 
      val.write(os, 16);
      return;
    case 'o':
    case 'O': 
      val.write(os, 8);
      return;
    case 's':
    case 'S': 
      os << val.to_string();
      return;
    case 'u':
    case 'U':
      val.write(os, 16);
      return;
    default: 
      break;
  } 
  char buffer[1024]; 
  std::snprintf(buffer, 1024, fmt, val.to_double());
  os << buffer;
}

} // namespace cascade

#endif
