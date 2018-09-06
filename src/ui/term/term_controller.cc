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

#include "src/ui/term/term_controller.h"

#include <iostream>
#include <limits>
#include <sys/select.h>
#include <unistd.h>
#include "src/runtime/runtime.h"
#include "src/verilog/parse/parser.h"

using namespace std;

namespace cascade {

TermController::TermController(Runtime* rt) : Controller(rt) { }

void TermController::run_logic() {
  while (!stop_requested()) {
    fd_set read_set;
    FD_ZERO(&read_set);
    struct timeval tv = {0, 100};

    while (!FD_ISSET(STDIN_FILENO, &read_set)) {
      FD_SET(STDIN_FILENO, &read_set);
      select(STDIN_FILENO+1, &read_set, nullptr, nullptr, &tv);
      if (stop_requested()) {
        return;
      }
    }

    Parser p;
    auto res = p.parse(cin);
    if (res.second) {
      runtime()->display("Caught Ctrl-D. Exiting...");
      runtime()->finish(0);
      break;
    } else if (p.error()) {
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
      runtime()->error("Parse Error:\n" + p.what()); 
    } else {
      runtime()->eval(res.first);
    }
  }
}

} // namespace cascade
