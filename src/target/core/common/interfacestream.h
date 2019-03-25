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

#ifndef CASCADE_SRC_TARGET_CORE_COMMON_INTERFACESTREAM_H
#define CASCADE_SRC_TARGET_CORE_COMMON_INTERFACESTREAM_H

#include <cassert>
#include <iostream>
#include <streambuf>
#include "src/runtime/runtime.h"
#include "src/target/interface.h"

namespace cascade {

class interfacebuf : public std::streambuf {
  public:
    // Typedefs:
    typedef std::streambuf::char_type char_type;
    typedef std::streambuf::traits_type traits_type;
    typedef std::streambuf::int_type int_type;
    typedef std::streambuf::pos_type pos_type;
    typedef std::streambuf::off_type off_type;

    // Constructors:
    explicit interfacebuf(Interface* interface, SId id);
    ~interfacebuf() override = default; 

  private:
    // Positioning:
    pos_type seekpos(std::streambuf::pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
    std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
    int sync() override;

    // Get Area:
    std::streamsize showmanyc() override;
    int_type underflow() override;
    int_type uflow() override;
    std::streamsize xsgetn(char_type* s, std::streamsize count) override;

    // Put Area:
    std::streamsize xsputn(const char_type* s, std::streamsize count) override;
    int_type overflow(int_type c = traits_type::eof()) override;

    // Attributes:
    Interface* interface_;
    SId id_;
};

class interfacestream : public std::iostream {
  public:
    explicit interfacestream(Interface* interface, SId id);
    ~interfacestream() override = default;

  private:
    interfacebuf buf_;
};

interfacebuf::interfacebuf(Interface* interface, SId id) {
  interface_ = interface;
  id_ = id;
}

inline std::streambuf::pos_type interfacebuf::seekpos(std::streambuf::pos_type pos, std::ios_base::openmode which) {
  const auto res = interface_->pubseekpos(id_, pos, (which == std::ios_base::in));
  return static_cast<std::streambuf::pos_type>(static_cast<std::streambuf::off_type>(res));
}

inline std::streampos interfacebuf::seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which) {
  // TODO(eschkufz) Limited support for usage until it comes up that we need it
  assert(way == std::ios_base::cur);
  (void) way;
  const auto res = interface_->pubseekoff(id_, off, (which == std::ios_base::in));
  return static_cast<std::streambuf::pos_type>(static_cast<std::streambuf::off_type>(res));
}

inline int interfacebuf::sync() {
  return interface_->pubsync(id_);
}

inline std::streamsize interfacebuf::showmanyc() {
  return interface_->in_avail(id_);
}

inline std::streambuf::int_type interfacebuf::underflow() {
  return interface_->sgetc(id_);
}

inline std::streambuf::int_type interfacebuf::uflow() {
  return interface_->sbumpc(id_);
}

inline std::streamsize interfacebuf::xsgetn(char_type* s, std::streamsize count) {
  return interface_->sgetn(id_, s, count);
}

inline std::streamsize interfacebuf::xsputn(const char_type* s, std::streamsize count) {
  return interface_->sputn(id_, s, count);
}

inline std::streambuf::int_type interfacebuf::overflow(int_type c) {
  return interface_->sputc(id_, c);
}

inline interfacestream::interfacestream(Interface* interface, SId id) : std::iostream(&buf_), buf_(interface, id) { }

} // namespace cascade

#endif

