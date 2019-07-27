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

#ifndef CASCADE_SRC_COMMON_STREAM_H
#define CASCADE_SRC_COMMON_STREAM_H

#include <iostream>
#include <streambuf>
#include <string>

namespace cascade {

// This class is an instance of a delegating c++ ostream. It can be used to
// insert tab characters whenever a newline is emitted.

class indbuf : public std::streambuf {
  public:
    // Typedefs:
    typedef std::streambuf::char_type char_type;
    typedef std::streambuf::traits_type traits_type;
    typedef std::streambuf::int_type int_type;
    typedef std::streambuf::pos_type pos_type;
    typedef std::streambuf::off_type off_type;

    // Constructors:
    explicit indbuf(std::streambuf* buf);
    ~indbuf() override = default;

    void tab();
    void untab();
    void delim(const std::string& d);

  protected:
    // Minimal Delegate Interface:
    int_type overflow(int_type c = traits_type::eof()) override;
    int_type sync() override;

  private:
    std::streambuf* buf_;
    bool pending_;
    size_t depth_;
    std::string delim_;
};

class indstream : public std::ostream {
  public:
    explicit indstream(std::ostream& os);
    ~indstream() override = default;

    void tab();
    void untab();
    void delim(const std::string& d);

  private:
    indbuf buf_;
};

inline indbuf::indbuf(std::streambuf* buf) : buf_(buf), pending_(false), depth_(0), delim_("  ") { }

inline void indbuf::tab() {
  ++depth_;
}

inline void indbuf::untab() {
  --depth_;
}

inline void indbuf::delim(const std::string& d) {
  delim_ = d;
}

inline indbuf::int_type indbuf::overflow(int_type c) {
  switch (c) {
    case '\n':
    case '\r':
      pending_ = true;
      return buf_->sputc(c);
    default:
      if (pending_) {
        pending_ = false;
        for (size_t i = 0; i < depth_; ++i) {
          for (auto d : delim_) {
            buf_->sputc(d);
          }
        }
      }
    return buf_->sputc(c);
  }
}

inline indbuf::int_type indbuf::sync() {
  return buf_->pubsync();
}

inline indstream::indstream(std::ostream& os) : std::ostream(&buf_), buf_(os.rdbuf()) { }

inline void indstream::tab() {
  buf_.tab();
}

inline void indstream::untab() {
  buf_.untab();
}

inline void indstream::delim(const std::string& d) {
  buf_.delim(d);
}

} // namespace cascade

#endif
