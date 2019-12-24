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

#ifndef CASCADE_SRC_TARGET_CORE_AVMM_ULX3S_ULX3S_LOGIC_H
#define CASCADE_SRC_TARGET_CORE_AVMM_ULX3S_ULX3S_LOGIC_H

#include <functional>
#include <unistd.h>
#include "target/core/avmm/avmm_logic.h"
#include "target/core/avmm/ulx3s/ulx3s_compiler.h"
#include "target/core/avmm/ulx3s/ulx3s_logic.h"

namespace cascade::avmm {

template <size_t V, typename A, typename T>
class Ulx3sLogic : public AvmmLogic<V,A,T> {
  public:
    // Typedefs:
    typedef std::function<void()> Callback;

    Ulx3sLogic(Interface* interface, ModuleDeclaration* md, size_t slot);
    virtual ~Ulx3sLogic() override;

    Ulx3sLogic& set_fd(int fd);
    Ulx3sLogic& set_callback(Callback cb);

  private:
    int fd_;
    Callback cb_;

    void write_all(const char* data, size_t len);
    void read_all(char* data, size_t len);
};

template <size_t V, typename A, typename T>
inline Ulx3sLogic<V,A,T>::Ulx3sLogic(Interface* interface, ModuleDeclaration* md, size_t slot) : AvmmLogic<V,A,T>(interface, md, slot) { 
  fd_ = -1;
  cb_ = nullptr;

  AvmmLogic<V,A,T>::get_table()->set_read([this](A index) {
    T res = 0;
    A addr = (A(0) << std::numeric_limits<A>::digits-1) | index;

    // TODO(eschkufz) Ordering is correct... for little-endian machines, anyway
    write_all(reinterpret_cast<char*>(&res), sizeof(T));
    write_all(reinterpret_cast<char*>(&addr), sizeof(A));
    read_all(reinterpret_cast<char*>(&res), sizeof(T));

    return res;
  });
  AvmmLogic<V,A,T>::get_table()->set_write([this](A index, T val) {
    A addr = (A(1) << std::numeric_limits<A>::digits-1) | index;
    T res = 0;

    // TODO(eschkufz) Ordering is correct... for little-endian machines, anyway
    write_all(reinterpret_cast<char*>(&val), sizeof(T));
    write_all(reinterpret_cast<char*>(&addr), sizeof(A));
    read_all(reinterpret_cast<char*>(&res), sizeof(T));
  });
}

template <size_t V, typename A, typename T>
inline Ulx3sLogic<V,A,T>::~Ulx3sLogic() {
  if (cb_ != nullptr) {
    cb_();
  }
}

template <size_t V, typename A, typename T>
inline Ulx3sLogic<V,A,T>& Ulx3sLogic<V,A,T>::set_fd(int fd) {
  fd_ = fd;
  return *this;
}

template <size_t V, typename A, typename T>
inline Ulx3sLogic<V,A,T>& Ulx3sLogic<V,A,T>::set_callback(Callback cb) {
  cb_ = cb;
  return *this;
}

template <size_t V, typename A, typename T>
inline void Ulx3sLogic<V,A,T>::write_all(const char* data, size_t len) {
  for (size_t i = 0; i < len; i += write(fd_, data+i, len-i));
}

template <size_t V, typename A, typename T>
inline void Ulx3sLogic<V,A,T>::read_all(char* data, size_t len) {
  for (size_t i = 0; i < len; i += read(fd_, data+i, len-i));
}

} // namespace cascade::avmm

#endif
