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

#ifndef CASCADE_SRC_BASE_STREAM_BUFSTREAM_H
#define CASCADE_SRC_BASE_STREAM_BUFSTREAM_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <streambuf>

namespace cascade {

// This class provides a c++ stream interface to a resizable character buffer
// which is located in the heap. 

class bufbuf : public std::streambuf {
  public:
    // Typedefs:
    typedef std::streambuf::char_type char_type;
    typedef std::streambuf::traits_type traits_type;
    typedef std::streambuf::int_type int_type;
    typedef std::streambuf::pos_type pos_type;
    typedef std::streambuf::off_type off_type;

    // Constructors:
    bufbuf(size_t c);
    ~bufbuf() override;

    // Data Inspection:
    char* data();
    const char* data() const;

    // Size Inspection:
    void resize(size_t n);
    void reserve(size_t n);
    size_t size() const;
    size_t capacity() const;

  private:
    // Positioning:
    pos_type seekpos(std::streambuf::pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
    std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;

    // Get Area:
    std::streamsize showmanyc() override;
    int_type underflow() override;
    int_type uflow() override;
    std::streamsize xsgetn(char_type* s, std::streamsize count) override;

    // Put Area:
    std::streamsize xsputn(const char_type* s, std::streamsize count) override;
    int_type overflow(int_type c = traits_type::eof()) override;
};

class bufstream : public std::iostream {
  public:
    // Constructors:
    bufstream(size_t c = 1);
    ~bufstream() override = default;

    // Data Inspection:
    char* data();
    const char* data() const;

    // Size Inspection:
    void resize(size_t n);
    void reserve(size_t n);
    size_t size() const;
    size_t capacity() const;

  private:
    bufbuf buf_;
};

inline bufbuf::bufbuf(size_t c) {
  auto ptr_ = new char[c];
  setp(ptr_, ptr_+c);
  setg(ptr_, ptr_, ptr_);
}

inline bufbuf::~bufbuf() {
  delete[] pbase();
}

inline char* bufbuf::data() {
  return pbase();
}

inline const char* bufbuf::data() const {
  return pbase();
}

inline void bufbuf::resize(size_t n) {
  if (n > capacity()) {
    reserve(2*n);
  }
  pbump(n-(pptr()-pbase()));
  setg(pbase(), pbase(), pptr());
}

inline void bufbuf::reserve(size_t n) {
  if (n <= capacity()) {
    return;
  }

  auto nptr = new char[n];
  std::copy(pbase(), pptr(), nptr);
  delete[] pbase();

  const auto poff = pptr() - pbase();
  const auto goff = gptr() - eback();

  setp(nptr, nptr+n);
  pbump(poff);
  setg(nptr, nptr+goff, nptr+poff);
}

inline size_t bufbuf::size() const {
  return pptr()-pbase();
}

inline size_t bufbuf::capacity() const {
  return epptr()-pbase();
}

inline std::streambuf::pos_type bufbuf::seekpos(std::streambuf::pos_type pos, std::ios_base::openmode which) {
  if (which == std::ios_base::out) {
    pbump(pbase() + pos - pptr());
    return pptr()-pbase();
  } 
  if (which == std::ios_base::in) {
    gbump(eback() + pos - gptr());
    return gptr()-eback();
  } 
  return std::streambuf::pos_type(std::streambuf::off_type(-1));
}

inline std::streampos bufbuf::seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which) {
  if (which == std::ios_base::out) {
    const auto target = off + (
      way == std::ios_base::beg ? pbase() : 
      way == std::ios_base::cur ? pptr() : 
      epptr()
    );
    pbump(target - pptr());
    return pptr()-pbase();
  } 
  if (which == std::ios_base::in) {
    const auto target = off + (
      way == std::ios_base::beg ? eback() : 
      way == std::ios_base::cur ? gptr() : 
      egptr()
    );
    gbump(target - gptr());
    return gptr()-eback();
  } 
  return std::streambuf::pos_type(std::streambuf::off_type(-1));
}

inline std::streamsize bufbuf::showmanyc() {
  return egptr() - gptr();
}

inline std::streambuf::int_type bufbuf::underflow() {
  return traits_type::eof();
}

inline std::streambuf::int_type bufbuf::uflow() {
  return traits_type::eof();
}

inline std::streamsize bufbuf::xsgetn(char_type* s, std::streamsize count) {
  const auto cnt = std::min(count, showmanyc());
  std::copy(gptr(), gptr()+cnt, s);
  gbump(cnt);
  return cnt;
}

inline std::streamsize bufbuf::xsputn(const char_type* s, std::streamsize count) {
  if ((pptr()+count) >= epptr()) {
    reserve(2*(pptr()+count-pbase()));
  }
  std::copy(s, s+count, pptr());
  setg(eback(), gptr(), egptr()+count);
  pbump(count);
  return count;
}

inline std::streambuf::int_type bufbuf::overflow(int_type c) {
  if (pptr() == epptr()) {
    reserve(2*capacity());
  }
  if (c != traits_type::eof()) {
    *pptr() = c;
    pbump(1);
    setg(eback(), gptr(), egptr()+1);
  }
  return c;
}

inline bufstream::bufstream(size_t c) : std::iostream(&buf_), buf_(c) { }

inline char* bufstream::data() {
  return buf_.data();
}

inline const char* bufstream::data() const {
  return buf_.data();
}

inline void bufstream::resize(size_t n) {
  buf_.resize(n);
}

inline void bufstream::reserve(size_t n) {
  buf_.reserve(n);
}

inline size_t bufstream::size() const {
  return buf_.size();
}

inline size_t bufstream::capacity() const {
  return buf_.capacity();
}

} // namespace cascade

#endif
