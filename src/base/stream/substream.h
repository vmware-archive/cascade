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

#ifndef SRC_BASE_STREAM_SUBSTREAM_H
#define SRC_BASE_STREAM_SUBSTREAM_H

#include <iostream>
#include <streambuf>
#include <string>
#include <unordered_map>

namespace cascade {

// This class is an instanceof a delegating c++ ostream. It can be used to
// replace characters with alternate values as they are emitted.

class subbuf : public std::streambuf {
  public:
    // Typedefs:
    typedef std::streambuf::char_type char_type;
    typedef std::streambuf::traits_type traits_type;
    typedef std::streambuf::int_type int_type;
    typedef std::streambuf::pos_type pos_type;
    typedef std::streambuf::off_type off_type;

    // Constructors:
    explicit subbuf(std::streambuf* buf);
    ~subbuf() override = default;

    void recurse(bool r = true);
    void sub(char c, char s);
    void sub(char c, const std::string& s);

  protected:
    // Minimal Delegate Interface:
    int_type overflow(int_type c = traits_type::eof()) override;
    int_type sync() override;

  private:
    std::streambuf* buf_;
    std::unordered_map<char, std::string> subs_;
    bool recurse_;
};

class substream : public std::ostream {
  public:
    explicit substream(std::ostream& os);
    ~substream() override = default;

    void recurse(bool r = true); 
    void sub(char c, char s);
    void sub(char c, const std::string& s);

  private:
    subbuf buf_;
};

inline subbuf::subbuf(std::streambuf* buf) : buf_(buf), recurse_(false) { }

inline void subbuf::recurse(bool r) {
  recurse_ = r;
}

inline void subbuf::sub(char c, char s) {
  subs_[c] = {s};
}

inline void subbuf::sub(char c, const std::string& s) {
  subs_[c] = s;
}

inline subbuf::int_type subbuf::overflow(int_type c) {
  const auto itr = subs_.find(c);
  if (itr != subs_.end()) {
    const auto& str = itr->second;
    const auto len = str.length();
    for (size_t i = 0, ie = len-1; i < ie; ++i) {
      if (recurse_) {
        overflow(str[i]);
      } else {
        buf_->sputc(str[i]);
      }
    }
    return recurse_ ? overflow(str[len-1]) : buf_->sputc(str[len-1]);
  }
  return buf_->sputc(c);
}

inline subbuf::int_type subbuf::sync() {
  return buf_->pubsync();
}

inline substream::substream(std::ostream& os) : std::ostream(&buf_), buf_(os.rdbuf()) { }

inline void substream::recurse(bool r) {
  buf_.recurse(r);
}

inline void substream::sub(char c, char s) {
  buf_.sub(c, s);
}

inline void substream::sub(char c, const std::string& s) {
  buf_.sub(c, s);
}

} // namespace cascade

#endif
