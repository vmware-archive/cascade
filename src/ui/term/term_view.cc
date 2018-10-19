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

#include "src/ui/term/term_view.h"

#include <iostream>
#include "src/verilog/print/term/term_printer.h"
#include "src/verilog/program/program.h"

using namespace std;

namespace cascade {

TermView::TermView() : View() { 
  cout << ">>> ";
}

void TermView::error(size_t t, const string& s) {
  (void) t;
  lock_guard<mutex> lg(lock_);
  TermPrinter(cout) << Color::RED << s << Color::RESET << "\n";
  cout << ">>> ";
  cout.flush();
}

void TermView::print(size_t t, const string& s) {
  (void) t;
  lock_guard<mutex> lg(lock_);
  TermPrinter(cout) << s;
  cout << ">>> ";
  cout.flush();
}

void TermView::warn(size_t t, const string& s) {
  (void) t;
  lock_guard<mutex> lg(lock_);
  TermPrinter(cout) << Color::YELLOW << s << Color::RESET << "\n";
  cout << ">>> ";
  cout.flush();
}

void TermView::eval_decl(size_t t, const Program* p, const ModuleDeclaration* md) {
  (void) t;
  (void) p;
  (void) md;

  lock_guard<mutex> lg(lock_);
  TermPrinter(cout) << Color::GREEN << "DECL OK" << Color::RESET << "\n";
  cout << ">>> ";
  cout.flush();
}

void TermView::eval_item(size_t t, const Program* p, const ModuleDeclaration* md) {
  (void) t;
  (void) p;
  (void) md;

  lock_guard<mutex> lg(lock_);
  TermPrinter(cout) << Color::GREEN << "ITEM OK" << Color::RESET << "\n";
  cout << ">>> ";
  cout.flush();
}

void TermView::crash() {
  lock_guard<mutex> lg(lock_);
  TermPrinter(cout) << Color::RED << "CASCADE SHUTDOWN UNEXPECTEDLY --- PLEASE FORWARD LOG FILE TO DEVELOPERS" << Color::RESET << "\n";
  cout.flush();
}

} // namespace cascade
