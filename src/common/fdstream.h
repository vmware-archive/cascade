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

#ifndef CASCADE_SRC_COMMON_FDSTREAM_H
#define CASCADE_SRC_COMMON_FDSTREAM_H

#include <iostream>
#include <streambuf>
#include <sys/socket.h>
#include <vector>

namespace cascade {

// This class provides a c++ stream interface to *nix file descriptors. The
// implementation of this class uses a resizable character buffer in the heap
// to store character data in between calls to flush.

class fdbuf : public std::streambuf {
  public:
    // Typedefs:
    typedef std::streambuf::char_type char_type;
    typedef std::streambuf::traits_type traits_type;
    typedef std::streambuf::int_type int_type;
    typedef std::streambuf::pos_type pos_type;
    typedef std::streambuf::off_type off_type;
   
    // Constructors:
    explicit fdbuf(int fd);
    ~fdbuf() override = default;

  private:
    // File Descriptor
    int fd_;
    // Get/Input/Read Area
    std::vector<char_type> get_;
    // Put/Output/Write Area
    std::vector<char_type> put_;

    // Locales: 
    void imbue(const std::locale& loc) override;

    // Positioning:
    fdbuf* setbuf(char_type* s, std::streamsize n) override;
    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
    pos_type seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
    int sync() override;

    // Get Area:
    std::streamsize showmanyc() override;
    int_type underflow() override;
    int_type uflow() override;
    std::streamsize xsgetn(char_type* s, std::streamsize count) override;

    // Put Area:
    std::streamsize xsputn(const char_type* s, std::streamsize count) override;
    int_type overflow(int_type c = traits_type::eof()) override;

    // Undo:
    int_type pbackfail(int_type c = traits_type::eof()) override;

    // Send/Recv:
    int send(const char_type* c, size_t len);
    int recv(char_type* c, size_t len);
};

class ifdstream : public std::istream {
  public:
    ifdstream(int fd);
    ~ifdstream() override = default;

  private:
    fdbuf buf_;
};

class ofdstream : public std::ostream {
  public:
    ofdstream(int fd);
    ~ofdstream() override = default;

  private:
    fdbuf buf_;
};

class fdstream : public std::iostream {
  public:
    fdstream(int fd);
    ~fdstream() override = default;

  private:
    fdbuf buf_;
};

inline fdbuf::fdbuf(int fd) : get_(1), put_(1) {
  fd_ = fd;
  setg(get_.data(), get_.data(), get_.data());
  setp(put_.data(), put_.data()+1);
}

inline void fdbuf::imbue(const std::locale& loc) {
  // Does nothing.
  (void) loc;
}

inline fdbuf* fdbuf::setbuf(char_type* s, std::streamsize n) {
  // Does nothing.
  (void) s;
  (void) n;
  return this;
}

inline fdbuf::pos_type fdbuf::seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) {
  // Does nothing.
  (void) off;
  (void) dir;
  (void) which;
  return pos_type(off_type(-1));
}

inline fdbuf::pos_type fdbuf::seekpos(pos_type pos, std::ios_base::openmode which) {
  (void) pos;
  (void) which;
  return pos_type(off_type(-1));
}

inline int fdbuf::sync() {
  const uint32_t n = pptr()-pbase();
  if (n == 0) {
    return 0;
  } else if (send((const char*)&n, sizeof(n)) == -1) {
    return -1;
  } else if (send((const char*)pbase(), n) == -1) {
    return -1;
  }

  setp(put_.data(), put_.data()+put_.size());
  return 0;
}

inline std::streamsize fdbuf::showmanyc() {
  return egptr() - gptr();
}

inline fdbuf::int_type fdbuf::underflow() {
  uint32_t n = 0;
  if (recv((char_type*)&n, sizeof(n)) == -1) {
    return traits_type::eof(); 
  }
  if (n > get_.size()) {
    get_.resize(n);
  }
  if (recv((char_type*)get_.data(), n) == -1) {
    return traits_type::eof();
  }
  setg(get_.data(), get_.data(), get_.data()+n);
  return traits_type::to_int_type(get_[0]);
}

inline fdbuf::int_type fdbuf::uflow() {
  uint32_t n = 0;
  if (recv((char_type*)&n, sizeof(n)) == -1) {
    return traits_type::eof(); 
  }
  if (n > get_.size()) {
    get_.resize(n);
  }
  if (recv((char_type*)get_.data(), n) == -1) {
    return traits_type::eof();
  }
  setg(get_.data(), get_.data()+1, get_.data()+n);
  return traits_type::to_int_type(get_[0]);
}

inline std::streamsize fdbuf::xsgetn(char_type* s, std::streamsize count) {
  std::streamsize total = 0;
  std::streamsize chunk = 0;
  do {
    chunk = std::min(egptr()-gptr(), count);
    std::copy(gptr(), gptr()+chunk, s+total);
    total += chunk;
  } 
  while ((total < count) && (underflow() != -1));
  setg(eback(), gptr()+chunk, egptr());
  return total;
}

inline std::streamsize fdbuf::xsputn(const char_type* s, std::streamsize count) {
  const size_t n = pptr()-pbase();
  const size_t req = n+count;
  if (req > put_.size()) {
    put_.resize(req);
  }

  std::copy(s, s+count, put_.data()+n);
  setp(put_.data(), put_.data()+put_.size());
  pbump(n+count);

  return count;
}

inline fdbuf::int_type fdbuf::overflow(int_type c) {
  if (c == traits_type::eof()) {
    return traits_type::to_int_type('0');
  }
  const auto n = put_.size();
  put_.resize(2*put_.size());
  put_[n] = c;
  setp(put_.data(), put_.data()+put_.size());
  pbump(n+1);

  return traits_type::to_int_type(c);
}

inline fdbuf::int_type fdbuf::pbackfail(int_type c) {
  if (gptr() == eback()) {
    return traits_type::eof();
  }
  gbump(-1);
  *gptr() = c;
  return traits_type::to_int_type(c);
}

inline int fdbuf::send(const char_type* c, size_t len) {
  int total = 0;
  while (total < (int)len) {
    const auto res = ::send(fd_, c+total, len-total, 0);
    if (res == -1) {
      return -1;
    }
    total += res;
  }
  return total;
}

inline int fdbuf::recv(char_type* c, size_t len) {
  int total = 0;
  while (total < (int)len) {
    const auto res = ::recv(fd_, c+total, len-total, 0);
    if (res == -1) {
      return -1;
    }
    total += res;
  }
  return total;
}

inline ifdstream::ifdstream(int fd) : std::istream(&buf_), buf_(fd) { }

inline ofdstream::ofdstream(int fd) : std::ostream(&buf_), buf_(fd) { }

inline fdstream::fdstream(int fd) : std::iostream(&buf_), buf_(fd) { }

} // namespace cascade

#endif
