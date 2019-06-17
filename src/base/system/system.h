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

#ifndef CASCADE_SRC_BASE_SYSTEM_SYSTEM_H
#define CASCADE_SRC_BASE_SYSTEM_SYSTEM_H

#include <cstdlib>
#include <string>
#ifdef __APPLE__
#include <libproc.h>
#endif
#include <unistd.h>

namespace cascade {

// This class is a thin wrapper aorund some basic *nix environment
// functionality. Its goal is to abstract away platform specific differences
// between osx and linux.

struct System {
  static std::string src_root();
  static int execute(const std::string& cmd);
};

inline std::string System::src_root() {
  char result[1024];
  #ifdef __APPLE__
    const auto pid = getpid();
    const auto count = proc_pidpath(pid, result, 1024);
  #else
    const auto count = readlink("/proc/self/exe", result, 1024);
  #endif
  const auto path = std::string(result, (count > 0) ? count : 0);

  return path.substr(0, path.rfind('/')) + "/../..";
}

inline int System::execute(const std::string& cmd) {
  return ::system(cmd.c_str());
}

} // namespace cascade

#endif
