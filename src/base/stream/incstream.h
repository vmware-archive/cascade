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

#ifndef CASCADE_SRC_BASE_STREAM_INCSTREAM_H
#define CASCADE_SRC_BASE_STREAM_INCSTREAM_H

#include <fstream>
#include <string>
#include <vector>

namespace cascade {

// This class is a wrapper around the c++ standard library's ifstream.  It adds
// support for specifying multiple search paths for the filename which is
// provided when the stream is created and the open method is called.

class incstream : public std::ifstream {
  public:
    incstream();
    incstream(const std::string& dirs);
    virtual ~incstream() = default;

    bool open(const std::string& path);

  private:
    std::vector<std::string> dirs_;

    void read_dirs(const std::string& dirs);
};

inline incstream::incstream() : std::ifstream() {
  read_dirs(".");
}

inline incstream::incstream(const std::string& dirs) : std::ifstream() {
  read_dirs(".");
  read_dirs(dirs);
}

inline bool incstream::open(const std::string& path) {
  for (const auto& d : dirs_) {
    std::ifstream::open(d + "/" + path);
    if (is_open()) {
      return true;
    }
    close();
  }
  return false;
}

inline void incstream::read_dirs(const std::string& dirs) {
  const auto dd = dirs + ":";
  for (size_t i = 0, j = 0; j != dd.length()-1; i = j+1) {
    j = dd.find_first_of(':', i);
    const auto dir = dd.substr(i, j-i);
    if (dir.length() > 0) {
      dirs_.push_back(dir);
    }
  }
}

} // namespace cascade

#endif
