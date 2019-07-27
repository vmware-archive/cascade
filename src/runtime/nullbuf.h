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

#ifndef CASCADE_SRC_RUNTIME_NULLBUF_H
#define CASCADE_SRC_RUNTIME_NULLBUF_H

#include <streambuf>

namespace cascade {

class nullbuf : public std::streambuf {
  public:
    // Typedefs:
    typedef std::streambuf::char_type char_type;
    typedef std::streambuf::traits_type traits_type;
    typedef std::streambuf::int_type int_type;
    typedef std::streambuf::pos_type pos_type;
    typedef std::streambuf::off_type off_type;
   
    // Constructors:
    ~nullbuf() override = default;

  private:
    std::streamsize xsputn(const char_type* s, std::streamsize count) override;
    int_type overflow(int_type c = traits_type::eof()) override;

    int_type underflow() override;
    int_type uflow() override;
    std::streamsize xsgetn(char_type* s, std::streamsize count) override;
};

inline std::streamsize nullbuf::xsputn(const char_type* s, std::streamsize count) {
  (void) s;
  return count;
}

inline nullbuf::int_type nullbuf::overflow(int_type c) {
  return c;
}

inline nullbuf::int_type nullbuf::underflow() {
  return traits_type::eof();
}

inline nullbuf::int_type nullbuf::uflow() {
  return traits_type::eof();
}

inline std::streamsize nullbuf::xsgetn(char_type* s, std::streamsize count) {
  (void) s;
  (void) count;
  return 0;
}

} // namespace 

#endif
