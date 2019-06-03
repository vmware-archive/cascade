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

#ifndef CASCADE_SRC_CL_COMMENT_STREAM_H
#define CASCADE_SRC_CL_COMMENT_STREAM_H

#include <iostream>
#include <streambuf>

namespace cascade::cl {

class comment_buf : public std::streambuf {
  public:
    comment_buf(std::streambuf* buf) : std::streambuf(), buf_(buf), on_comment_(false) {}

  protected:
    int_type underflow() override {
      auto res = buf_->sgetc();
      if (res != '#') {
        return on_comment_ ? traits_type::to_int_type(' ') : res;
      }
      do {
        res = buf_->snextc();
      } while (res != traits_type::eof() && res != '\n');
      on_comment_ = res == '\n';
      return on_comment_ ? traits_type::to_int_type(' ') : res; 
    }
    int_type uflow() override {
      const auto res = underflow();
      if (res != traits_type::eof()) {
        buf_->sbumpc();
        on_comment_ = false;
      }
      return res;
    }

  private:
    std::streambuf* buf_;
    bool on_comment_;
};

class comment_stream : public std::istream {
  public:
    comment_stream(std::istream& is) : std::istream(&buf_), buf_(is.rdbuf()) {}
  private:
    comment_buf buf_;
};

} // namespace cascade::cl

#endif
