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

#include "src/ui/log/log_view.h"

#include <ctime>
#include "src/verilog/ast/ast.h"
#include "src/verilog/print/text/text_printer.h"
#include "src/verilog/program/program.h"

using namespace std;

namespace cascade {

LogView::LogView(ostream& os) : View(), os_(os) { }

void LogView::startup(size_t t) {
  os_ << "STARTUP " << t << " " << time(nullptr) << endl;
}

void LogView::shutdown(size_t t) {
  os_ << "SHUTDOWN " << t << " " << time(nullptr) << endl;
}

void LogView::error(size_t t, const string& s) {
  os_ << "ERROR " << t << " " << time(nullptr) << endl << s << endl;
}

void LogView::print(size_t t, const string& s) {
  os_ << "PRINT " << t << " " << time(nullptr) << endl << s << endl;
}

void LogView::warn(size_t t, const string& s) {
  os_ << "WARN " << t << " " << time(nullptr) << endl << s << endl;
}

void LogView::parse(size_t t, size_t d, const string& s) {
  (void) d;
  os_ << "PARSE " << t << " " << time(nullptr) << endl << s << endl;
}

void LogView::include(size_t t, const string& s) {
  os_ << "INCLUDE " << t << " " << time(nullptr) << endl << s << endl;
}

void LogView::decl(size_t t, const Program* p, const ModuleDeclaration* md) {
  (void) p;
  os_ << "DECL " << t << " " << time(nullptr) << endl;
  TextPrinter(os_) << md << "\n";
}

void LogView::item(size_t t, const Program* p, const ModuleDeclaration* md) {
  (void) p;
  os_ << "ITEM " << t << " " << time(nullptr) << endl;
  TextPrinter(os_) << md << "\n";
}

void LogView::crash() {
  os_ << "CRASH " << time(nullptr) << endl;
}

} // namespace cascade
