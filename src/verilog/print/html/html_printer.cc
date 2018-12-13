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

#include "src/verilog/print/html/html_printer.h"

using namespace std;

namespace cascade {

HtmlPrinter::HtmlPrinter(ostream& os) : Printer(*init(os)) { }

HtmlPrinter::~HtmlPrinter() {
  delete ss_;
}

string HtmlPrinter::reset() {
  return "</span>";
}

string HtmlPrinter::red() {
  return "<span style='color:red'>";
}

string HtmlPrinter::green() {
  return "<span style='font-weight:bold; color:green'>";
}

string HtmlPrinter::yellow() {
  return "<span style='font-weight:bold; color:orange'>";
}

string HtmlPrinter::blue() {
  return "<span style='color:blue'>";
}

string HtmlPrinter::grey() {
  return "<span style='color:grey'>";
}

substream* HtmlPrinter::init(ostream& os) {
  ss_ = new substream(os);
  ss_->sub('"', "&quot;");
  ss_->sub('\t', "&nbsp;&nbsp;");
  ss_->sub('\n', "<br/>");
  return ss_;  
}

} // namespace cascade
