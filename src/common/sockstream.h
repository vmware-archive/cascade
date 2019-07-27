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

#ifndef CASCADE_SRC_COMMON_SOCKSTREAM_H
#define CASCADE_SRC_COMMON_SOCKSTREAM_H

#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <resolv.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "common/fdstream.h"

namespace cascade {

// This class is a specialized instance of an fdsrream which is used to
// encapsulate the behavior of a TCP or UNIX Domain socket. 

class sockstream : public fdstream {
  public:
    sockstream(int fd);
    sockstream(const char* path);
    sockstream(const char* host, uint32_t port);
    ~sockstream() override;

    // Returns true if an error occurred while creating this socket
    bool error() const;
    // Returns true if the file descriptor underlying this socket is valid
    bool valid() const;
    // Returns the file descriptor underlying this socket
    int descriptor() const;

  private:
    int raw_fd(int fd);
    int unix_sock(const char* path);
    int inet_sock(const char* host, uint32_t port);
    int fd_;
};

inline sockstream::sockstream(int fd) : fdstream(raw_fd(fd)) { }

inline sockstream::sockstream(const char* path) : fdstream(unix_sock(path)) { }

inline sockstream::sockstream(const char* host, uint32_t port) : fdstream(inet_sock(host, port)) { }

inline sockstream::~sockstream() {
  if (fd_ != -1) {
    flush();
    ::close(fd_);
  }
}

inline bool sockstream::error() const {
  return fd_ == -1;
}

inline bool sockstream::valid() const {
  errno = 0;
  return (fcntl(fd_, F_GETFD) != -1) || (errno != EBADF);
}

inline int sockstream::descriptor() const {
  return fd_;
}

inline int sockstream::raw_fd(int fd) {
  fd_ = fd;
  return fd_;
}

inline int sockstream::unix_sock(const char* path) {
  struct sockaddr_un dest;
  bzero(&dest, sizeof(dest));
  dest.sun_family = AF_UNIX;
  strncpy(dest.sun_path, path, sizeof(dest.sun_path)-1);

  fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
  if ((fd_ != -1) && (::connect(fd_, (struct sockaddr*)&dest, sizeof(dest)) != 0)) {
    fd_ = -1;
  } 
  return fd_;
}

inline int sockstream::inet_sock(const char* host, uint32_t port) {
  struct sockaddr_in dest;
  bzero(&dest, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(port);
  inet_aton(host, (in_addr*) &dest.sin_addr.s_addr);

  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if ((fd_ != -1) && (::connect(fd_, (struct sockaddr*)&dest, sizeof(dest)) != 0)) {
    fd_ = -1;
  } 
  return fd_;
}

} // namespace cascade

#endif
