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

#ifndef SRC_BASE_STREAM_CACHESTREAM_H
#define SRC_BASE_STREAM_CACHESTREAM_H

#include <iostream>
#include <streambuf>

namespace cascade {

// This class interposes a cache in front of a stream. This class may be useful
// for high-latency streams where repeated invocations of sgetc() and sputc()
// are slower than bulk transfer using single invocations of sgetn() and
// sputn().

class cachebuf : public std::streambuf {
  public:
    // Typedefs:
    typedef std::streambuf::char_type char_type;
    typedef std::streambuf::traits_type traits_type;
    typedef std::streambuf::int_type int_type;
    typedef std::streambuf::pos_type pos_type;
    typedef std::streambuf::off_type off_type;
   
    // Constructors:
    explicit cachebuf(std::streambuf* backend, size_t n = 1024);
    ~cachebuf() override;

  private:
    // Get/Input/Read Area
    std::vector<char_type> get_;
    // Put/Output/Write Area
    std::vector<char_type> put_;
    // Backend store
    std::streambuf* backend_;

    // Locales: 
    void imbue(const std::locale& loc) override;

    // Positioning:
    cachebuf* setbuf(char_type* s, std::streamsize n) override;
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
    void flush_get();
    void flush_put();
};

inline cachebuf::cachebuf(std::streambuf* backend, size_t n) : get_(n), put_(n) {
  backend_ = backend;
  setg(get_.data(), get_.data()+get_.size(), get_.data()+get_.size());
  setp(put_.data(), put_.data()+put_.size());
}

inline cachebuf::~cachebuf() {
  flush_get();
  flush_put();
}

inline void cachebuf::imbue(const std::locale& loc) {
  // Does nothing.
  (void) loc;
}

inline cachebuf* cachebuf::setbuf(char_type* s, std::streamsize n) {
  // Does nothing.
  (void) s;
  (void) n;
  return this;
}

inline cachebuf::pos_type cachebuf::seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) {
  // Synchronize whichever areas we're about to change the position of
  if (which == std::ios_base::in) {
    flush_get();
  } 
  if (which == std::ios_base::out) {
    flush_put();
  }
  // Now delegate the request to the backend
  return backend_->pubseekoff(off, dir, which);
}

inline cachebuf::pos_type cachebuf::seekpos(pos_type pos, std::ios_base::openmode which) {
  // Synchronize whichever areas we're about to change the position of
  if (which == std::ios_base::in) {
    flush_get();
  } 
  if (which == std::ios_base::out) {
    flush_put();
  }
  // Now delegate the request to the backend
  return backend_->pubseekpos(pos, which);
}

inline int cachebuf::sync() {
  // Flush the buffers and sync the backend
  flush_get();
  flush_put();
  return backend_->pubsync();
}

inline std::streamsize cachebuf::showmanyc() {
  // If there are bytes in the get area, we can return at least that many.
  // Otherwise, it's up to the backend to produce an estimate.
  return (egptr() > gptr()) ? (egptr() - gptr()) : backend_->in_avail();
}

inline cachebuf::int_type cachebuf::underflow() {
  // Refill the get area with as much as we can hold
  const auto n = backend_->sgetn(get_.data(), get_.size());
  if (n == 0) {
    return traits_type::eof(); 
  }
  // Return the first element of the get area and don't increment the get pointer
  setg(get_.data(), get_.data(), get_.data()+n);
  return traits_type::to_int_type(get_[0]);
}

inline cachebuf::int_type cachebuf::uflow() {
  // Refill the get area with as much as we can hold
  const auto n = backend_->sgetn(get_.data(), get_.size());
  if (n == 0) {
    return traits_type::eof(); 
  }
  // Return the first element of the get area and increment the get pointer
  setg(get_.data(), get_.data()+1, get_.data()+n);
  return traits_type::to_int_type(get_[0]);
}

inline std::streamsize cachebuf::xsgetn(char_type* s, std::streamsize count) {
  // How many bytes are left in the get area?
  const auto n = egptr()-gptr();

  // If we have enough, handle this read in the get area
  if (n >= count) {
    std::copy(gptr(), gptr()+count, s);
    gbump(count);
    return count;
  }

  // Otherwise, use whatever is in the get area and then handle the rest of the
  // request directly from the backend (it doesn't make sense to cache
  // something this big).
  std::copy(gptr(), egptr(), s);
  backend_->sgetn(s+n, count-n);
  gbump(n);

  return count;
}

inline std::streamsize cachebuf::xsputn(const char_type* s, std::streamsize count) {
  // How many bytes are left in the put area?
  const auto n = epptr()-pptr();

  // If we have enough, handle this write in the put area.
  if (n >= count) {
    std::copy(s, s+count, pptr());
    pbump(count);
    return count;
  }

  // Otherwise, flush whatever is in the put area and write directly to the
  // backend (it doesn't make sense to cache anything this big).
  flush_put();
  return backend_->sputn(s, count);
}

inline cachebuf::int_type cachebuf::overflow(int_type c) {
  // We're full. Flush the put area.
  flush_put();

  // Add the character to the put area and update the put pointer.
  put_[0] = c;
  setp(put_.data(), put_.data()+put_.size());
  pbump(1);

  return traits_type::to_int_type(c);
}

inline cachebuf::int_type cachebuf::pbackfail(int_type c) {
  // If the get area is empty, fall back on the backend to try to deal with this.
  if (gptr() == eback()) {
    return backend_->sputbackc(c);
  }

  // Otherwise, just decrement the get pointer
  gbump(-1);
  *gptr() = c;
  return traits_type::to_int_type(c);
}

inline void cachebuf::flush_get() {
  if (egptr() == gptr()) {
    return;
  }
  backend_->pubseekoff(-(gptr()-eback()), std::ios_base::cur, std::ios_base::in);
  setg(get_.data(), get_.data()+get_.size(), get_.data()+get_.size());
}

inline void cachebuf::flush_put() {
  if (pptr() == pbase()) {
    return;
  }
  backend_->sputn(pbase(), pptr()-pbase());
  setp(put_.data(), put_.data()+put_.size());
}

} // namespace cascade

#endif

