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

#ifndef CASCADE_SRC_VERILOG_ANALYZE_PRINTF_H
#define CASCADE_SRC_VERILOG_ANALYZE_PRINTF_H

#include <sstream>
#include <string>
#include "src/verilog/analyze/evaluate.h"
#include "src/verilog/ast/ast.h"
#include "src/verilog/print/text/text_printer.h"

namespace cascade {

struct Printf {
  template <typename InputItr>
  std::string format(InputItr begin, InputItr end);
};

template <typename InputItr>
inline std::string Printf::format(InputItr begin, InputItr end) {
  if (begin == end) {
    return "";
  }

  std::stringstream ss;
  auto a = begin;

  auto s = dynamic_cast<const String*>(*a);
  if (s == nullptr) {
    Evaluate().get_value(*a).write(ss, 10);
    return ss.str();
  } 

  for (size_t i = 0, j = 0; ; i = j+2) {
    j = s->get_readable_val().find_first_of('%', i);
    TextPrinter(ss) << s->get_readable_val().substr(i, j-i);
    if (j == std::string::npos) {
      break;
    }
    if (++a == end) {
      continue;
    }
    switch (s->get_readable_val()[j+1]) {
      case 'b':
      case 'B': 
        Evaluate().get_value(*a).write(ss, 2);
        break;
      case 'd':
      case 'D':
        Evaluate().get_value(*a).write(ss, 10);
        break;
      case 'h':
      case 'H': 
        Evaluate().get_value(*a).write(ss, 16);
        break;
      case 'o':
      case 'O': 
        Evaluate().get_value(*a).write(ss, 8);
        break;
      default: 
        assert(false);
    }
  }
  return ss.str();
}

} // namespace cascade

#endif
