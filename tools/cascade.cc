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

#include <cstring>
#include <signal.h>
#include <fstream>
#include <sstream>
#include "base/system/system.h"
#include "cascade/cascade.h"
#include "cl/cl.h"

using namespace cascade;
using namespace cascade::cl;
using namespace std;

namespace {

__attribute__((unused)) auto& g1 = Group::create("Cascade Runtime Options");
auto& march = StrArg<string>::create("--march")
  .usage("minimal|minimal_jit|sw|de10|de10_jit")
  .description("Target architecture")
  .initial("minimal");
auto& inc_dirs = StrArg<string>::create("-I")
  .usage("<path1>:<path2>:...:<pathn>")
  .description("Paths to search for files on")
  .initial("");
auto& input_path = StrArg<string>::create("-e")
  .usage("path/to/file.v")
  .description("Read input from file");

__attribute__((unused)) auto& g2 = Group::create("Quartus Server Options");
auto& quartus_host = StrArg<string>::create("--quartus_host")
  .usage("<host>")
  .description("Location of quartus server")
  .initial("localhost");
auto& quartus_port = StrArg<uint32_t>::create("--quartus_port")
  .usage("<port>")
  .description("Location of quartus server")
  .initial(9900);

__attribute__((unused)) auto& g3 = Group::create("Logging Options");
auto& profile = StrArg<int>::create("--profile")
  .usage("<n>")
  .description("Number of seconds to wait between profiling events; setting n to zero disables profiling; only effective with --enable_log")
  .initial(0);
auto& enable_info = FlagArg::create("--enable_info")
  .description("Turn on info messages");
auto& disable_warning = FlagArg::create("--disable_warning")
  .description("Turn off warning messages");
auto& disable_error = FlagArg::create("--disable_error")
  .description("Turn off error messages");
auto& enable_log = FlagArg::create("--enable_log")
  .description("Prints debugging information to log file");

__attribute__((unused)) auto& g4 = Group::create("Optimization Options");
auto& disable_inlining = FlagArg::create("--disable_inlining")
  .description("Prevents cascade from inlining modules");
auto& open_loop_target = StrArg<size_t>::create("--open_loop_target")
  .usage("<n>")
  .description("Maximum number of seconds to run in open loop for before transferring control back to runtime")
  .initial(1);

class termbuf : public std::streambuf {
  public:
    typedef streambuf::char_type char_type;
    typedef streambuf::traits_type traits_type;
    typedef streambuf::int_type int_type;
    typedef streambuf::pos_type pos_type;
    typedef streambuf::off_type off_type;

    termbuf(streambuf* target, const string& color = "");
    ~termbuf() override = default;

  private:
    streambuf* target_;
    string color_;
    stringstream ss_;

    int_type overflow(int_type c = traits_type::eof()) override;
    int_type sync() override;
};

termbuf::termbuf(streambuf* target, const string& color) : streambuf() {
  target_ = target;
  color_ = color;
}

termbuf::int_type termbuf::overflow(int_type c) {
  ss_.put(c);
  if (c == '\n') {
    ss_ << ((color_ != "") ? "\033[00m" : "") << ">>> " << color_;
  }
  return c;
}

termbuf::int_type termbuf::sync() {
  ostream(target_) << color_ << ss_.str() << ((color_ != "") ? "\033[00m" : "");
  ostream(target_).flush();
  ss_.str(string());
  return 0;
}

class logbuf : public std::streambuf {
  public:
    typedef streambuf::char_type char_type;
    typedef streambuf::traits_type traits_type;
    typedef streambuf::int_type int_type;
    typedef streambuf::pos_type pos_type;
    typedef streambuf::off_type off_type;

    logbuf(bool verbose);
    ~logbuf() override = default;

  private:
    filebuf fb_;

    int_type overflow(int_type c = traits_type::eof()) override;
    int_type sync() override;
};

logbuf::logbuf(bool verbose) : streambuf() {
  if (verbose) {
    fb_.open("cascade.log", ios::app);
  }
}

logbuf::int_type logbuf::overflow(int_type c) {
  fb_.sputc(c);
  return c;
}

logbuf::int_type logbuf::sync() {
  fb_.pubsync();
  cout << ">>> ";
  return 0;
}

Cascade cascade_;

void int_handler(int sig) {
  (void) sig;
  ::cascade_.request_stop();
}

void segv_handler(int sig) {
  (void) sig;
  cerr << "\033[31mCASCADE SHUTDOWN UNEXPECTEDLY\033[00m" << endl;
  cerr << "\033[31mPlease rerun with --enable_log and forward log file to developers\033[00m" << endl;
  exit(1);
}

} // namespace

int main(int argc, char** argv) {
  // Parse command line
  Simple::read(argc, argv);

  { struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = ::segv_handler;
    sigaction(SIGSEGV, &action, nullptr);
  }
  { struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = ::int_handler;
    sigaction(SIGINT, &action, nullptr);
  }

  ::cascade_.set_include_dirs(::inc_dirs.value() + ":" + System::src_root());
  ::cascade_.set_enable_inlining(!::disable_inlining.value());
  ::cascade_.set_open_loop_target(::open_loop_target.value());
  ::cascade_.set_quartus_server(::quartus_host.value(), ::quartus_port.value());
  ::cascade_.set_profile_interval(::profile.value());

  ::cascade_.set_stdout(new termbuf(cout.rdbuf()));
  if (!::disable_error.value()) {
    ::cascade_.set_stderr(new termbuf(cerr.rdbuf(), "\033[31m"));
  }
  if (!::disable_warning.value()) {
    ::cascade_.set_stdwarn(new termbuf(cerr.rdbuf(), "\033[33m"));
  }
  if (::enable_info.value()) {
    ::cascade_.set_stdinfo(new termbuf(clog.rdbuf(), "\033[37m"));
  }
  ::cascade_.set_stdlog(new logbuf(::enable_log.value()));

  ::cascade_.run();
  if (::input_path.value() != "") {
    ::cascade_ << "`include \"data/march/" << ::march.value() << ".v\"\n"
               << "`include \"" << ::input_path.value() <<  "\"" << endl;
  } else {
    ::cascade_ << "`include \"data/march/" << ::march.value() << ".v\"" << endl;
  }
  ::cascade_.stop_now();
  ::cascade_.rdbuf(cin.rdbuf());
  ::cascade_.run();
  ::cascade_.wait_for_stop();

  if (!::cascade_.is_finished()) {
    cerr << "\033[31mCaught Signal\033[00m" << endl;
  }
  cout << "Goodbye!" << endl;

  return 0;
}
