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

#include "src/target/core/de10/program_boxer.h"

#include <sstream>
#include "src/base/stream/indstream.h"
#include "src/verilog/ast/ast.h"
#include "src/target/core/de10/de10_logic.h"
#include "src/target/core/de10/module_boxer.h"

using namespace std;

namespace cascade {

bool ProgramBoxer::push(MId mid, const ModuleDeclaration* md, const De10Logic* de) {
  // Count the number of lines in this module. Code size increases monotonically, which
  // means it should function sufficiently as a sequence number.
  const auto seq = md->size_items();

  // Reject push requests for code which is older than the repository.  
  auto itr = repo_.find(mid);
  if (itr != repo_.end()) {
    const auto rseq = itr->second.first;
    if (seq < rseq) {
      return false;
    }
  }

  // This request is fresh. box the logic and insert it into the repository. 
  const auto bmd = ModuleBoxer().box(mid, md, de);
  repo_.insert(make_pair(mid, make_pair(seq, bmd)));
  return true;
}

string ProgramBoxer::get() const {
  stringstream ss;
  indstream os(ss);

  // Module Declarations
  for (const auto& s : repo_) {
    os << s.second.second << endl;
    os << endl;
  }

  // Top-level Module
  os << "module program_logic(" << endl;
  os.tab();
  os << "input wire clk," << endl;
  os << "input wire reset," << endl;
  os << endl;
  os << "input wire[15:0]  s0_address," << endl;
  os << "input wire [1:0]  s0_byteenable," << endl;
  os << "input wire        s0_read," << endl;
  os << "input wire        s0_write," << endl;
  os << endl;
  os << "output reg [31:0] s0_readdata," << endl;
  os << "input  wire[31:0] s0_writedata," << endl;
  os << endl;
  os << "output reg        s0_waitrequest" << endl;
  os.untab();
  os << ");" << endl;
  os.tab();

  os << "// Unpack address into module id and variable id" << endl;
  os << "wire[13:0] __vid = s0_address[13:0];" << endl;
  os << "wire [1:0] __mid = s0_byteenable;" << endl;

  os << "// Module Instantiations:" << endl;
  for (const auto& s : repo_) {
    os << "wire[31:0] m" << s.first << "_out;" << endl;
    os << "wire       m" << s.first << "_wait;" << endl;
    os << "M" << s.first << " m" << s.first << "(" << endl;
    os.tab();
    os << ".__clk(clk)," << endl;
    os << ".__read((__mid == " << s.first << ") & s0_write)," << endl;
    os << ".__vid(__vid)," << endl;
    os << ".__in(s0_writedata)," << endl;
    os << ".__out(m" << s.first << "_out)," << endl;
    os << ".__wait(m" << s.first << "_wait)" << endl;
    os.untab();
    os << ");" << endl;
  }

  os << "// Output Demuxing:" << endl;
  os << "reg[31:0] rd;" << endl;
  os << "reg wr;" << endl;
  os << "always @(*) begin" << endl;
  os.tab();
  os << "case (__mid)" << endl;
  os.tab();
  for (const auto& s : repo_) {
    os << s.first << ": begin rd = m" << s.first << "_out; wr = m" << s.first << "_wait; end" << endl;
  }
  os << "default: begin rd = 0; wr = 0; end" << endl;
  os.untab();
  os << "endcase" << endl;
  os.untab();
  os << "end" << endl;

  os << "// Output Logic:" << endl;
  os << "always @(posedge clk) begin" << endl;
  os.tab();
  os << "s0_waitrequest <= (s0_read | s0_write) ? wr : 1'b1;" << endl;
  os << "s0_readdata <= rd;" << endl;
  os.untab();
  os << "end" << endl;

  os.untab();
  os << "endmodule";

  return ss.str();
}

} // namespace cascade
