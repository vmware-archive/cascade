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

#include <sstream>
#include <string>
#include "verilog/analyze/evaluate.h"
#include "verilog/ast/ast.h"

namespace cascade {

class Printf {
  public:
    Printf(Evaluate* eval);
    std::string format(const std::string& format, const Expression* expr);
  private:
    Evaluate* eval_;
};

inline Printf::Printf(Evaluate* eval) {
  eval_ = eval;
}

inline std::string Printf::format(const std::string& format, const Expression* expr) {
  if (format[0] != '%') {
    return format;
  }

  std::stringstream ss;
  switch (format[1]) {
    case 'b':
    case 'B': 
      eval_->get_value(expr).write(ss, 2);
      break;
    case 'd':
    case 'D':
      eval_->get_value(expr).write(ss, 10);
      break;
    case 'h':
    case 'H': 
      eval_->get_value(expr).write(ss, 16);
      break;
    case 'o':
    case 'O': 
      eval_->get_value(expr).write(ss, 8);
      break;
    case 'u':
    case 'U':
      eval_->get_value(expr).write(ss, 16);
      break;
    default: 
      assert(false);
  }
  return ss.str();
}

} // namespace cascade

#endif
