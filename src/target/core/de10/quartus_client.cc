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

#include "src/target/core/de10/quartus_client.h"

#include <cctype>
#include <sstream>
#include <string>
#include <unistd.h>
#include "src/base/socket/socket.h"
#include "src/base/stream/indstream.h"

using namespace std;

namespace cascade {

QuartusClient::QuartusClient() : Asynchronous() { 
  set_batch_window(3);
  set_host("localhost"); 
  set_port(9900);

  refresh_requested_ = false;
  error_ = true;
}

QuartusClient& QuartusClient::set_batch_window(size_t bw) {
  batch_window_ = bw;
  return *this;
}

QuartusClient& QuartusClient::set_host(const string& host) {
  host_ = host;
  return *this;
}

QuartusClient& QuartusClient::set_port(uint32_t port) {
  port_ = port;
  return *this;
}

bool QuartusClient::refresh(MId id, const string& src) {
  unique_lock<mutex> ul(lock_);

  // Update source, flag refresh request, and wait until it's been serviced
  src_[id] = src;
  for (refresh_requested_ = true; !error_ && refresh_requested_;) { 
    cv_.wait(ul);
  }
  return !error_;
}

void QuartusClient::sync() {
  stringstream ss;
  indstream os(ss);

  // Module Declarations
  for (const auto& s : src_) {
    os << s.second << endl;
    os << endl;
  }

  // Top-level Module
  os << "module program_logic(" << endl;
  os.tab();
  os << "input wire clk," << endl;
  os << "input wire reset," << endl;
  os << endl;
  os << "input wire[15:0]  s0_address," << endl;
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
  os << "wire[7:0] __vid = s0_address[13:6];" << endl;
  os << "wire[5:0] __mid = s0_address[5:0];" << endl;

  os << "// Module Instantiations:" << endl;
  for (const auto& s : src_) {
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
  os << "wire wr;";
  os << "always @(*) begin" << endl;
  os.tab();
  os << "case (__mid)" << endl;
  os.tab();
  for (const auto& s : src_) {
    os << s.first << ": begin s0_readdata = m" << s.first << "_out; wr = m" << s.first << "_wait; end" << endl;
  }
  os << "default: begin s0_readdata = 0; wr = 0; end" << endl;
  os.untab();
  os << "endcase" << endl;
  os.untab();
  os << "end" << endl;

  os << "// Wait Logic:" << endl;
  os << "always @(posedge clk) begin" << endl;
  os.tab();
  os << "s0_waitrequest <= ~s0_waitrequest ? 1'b1 : ((s0_read | s0_write) & ~wr);" << endl;
  os.untab();
  os << "end" << endl;

  os.untab();
  os << "endmodule";

  // Send to server; block on ACK
  Socket sock;
  sock.connect(host_, port_);
  if (sock.error()) {
    error_ = true;
    return;
  }

  uint32_t len = ss.str().length();
  sock.send(len);
  sock.send(ss.str().c_str(), len);
  sock.recv(error_);
}

void QuartusClient::run_logic() {
  for (error_ = false, refresh_requested_ = false; !error_ && !stop_requested(); sleep(batch_window_)) {
    lock_guard<mutex> lg(lock_);
    if (!refresh_requested_) {
      continue;
    }
    sync();
    refresh_requested_ = false;
    cv_.notify_all();
  }
  {
    lock_guard<mutex> lg(lock_);
    error_ = true;
    cv_.notify_all();
  }
}

} // namespace cascade
