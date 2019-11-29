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

#ifndef CASCADE_SRC_COMMON_SYSTEM_H
#define CASCADE_SRC_COMMON_SYSTEM_H

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace cascade {

// This class is a thin wrapper aorund some basic *nix environment
// functionality. Its goal is to abstract away platform specific differences
// between osx and linux.

struct System {
  static std::string src_root();
  static int execute(const std::string& cmd);
  static int no_block_execute(const std::string& cmd, bool verbose = false);
};

#ifdef __APPLE__
inline std::string System::src_root() {
  char buffer[1024];
  uint32_t count = 1024;
  const auto res = _NSGetExecutablePath(buffer, &count);
  const auto path = (res == -1) ? "" : std::string(buffer);
  return path.substr(0, path.rfind('/')) + "/../..";
}
#else
inline std::string System::src_root() {
  char buffer[1024];
  const auto count = readlink("/proc/self/exe", buffer, 1024);
  const auto path = std::string(buffer, (count > 0) ? count : 0);
  return path.substr(0, path.rfind('/')) + "/../..";
}
#endif

inline int System::execute(const std::string& cmd) {
  return std::system(cmd.c_str());
}

inline int System::no_block_execute(const std::string& cmd, bool verbose) {
  const auto pid = fork();
  if (pid == 0) {
    if (!verbose) {
      fclose(stdout);
      fclose(stderr); 
    }
    return execl("/bin/sh", "sh", "-c", cmd.c_str(), nullptr);
  } else {
    while (true) {
      int status;
      const auto w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
      if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
      } else if (WIFSIGNALED(status)) {
        return WTERMSIG(status);
      } else if (WIFSTOPPED(status)) {
        return WSTOPSIG(status);
      }
    } 
  }
}

} // namespace cascade

#endif
