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

#ifndef CASCADE_SRC_CL_DIR_ARG_H
#define CASCADE_SRC_CL_DIR_ARG_H

#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include "comment_stream.h"
#include "str_arg.h"

namespace cascade::cl {

template <typename T, typename R>
class DirReader {
  public:
    bool operator()(std::istream& is, T& t) const {
      std::stringstream ss;
      std::string path = "";
      is >> path;
      return walk(ss, path) ? R()(ss, t) : false;
    }
  
  private:
    bool walk(std::stringstream& ss, const std::string& dir) const {
      DIR* dp = opendir(dir.c_str());    
      if (dp == NULL) {
        return false;
      }
      while (dirent* de = readdir(dp)) {
        const std::string file = de->d_name;
        if (file == "." || file == "..") {
          continue;
        }
        const auto path = dir + "/" + file;
        struct stat filestat;
        if (stat(path.c_str(), &filestat)) {
          return false;
        } 

        if (S_ISDIR(filestat.st_mode)) {
          if (!walk(ss, path)) {
            return false;
          }
        } else {
          std::ifstream ifs(path);
          if (!ifs.is_open()) {
            return false;
          }
          comment_stream cs(ifs);
          ss << cs.rdbuf();
        }
      }
      return true;
    }
};

template <typename T, typename R = StrReader<T>, typename W = StrWriter<T>>
using DirArg = StrArg<T, DirReader<T, R>, W>;

} // namespace cascade::cl

#endif
