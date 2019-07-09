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

#ifndef CASCADE_SRC_COMMON_SOCKSERVER_H
#define CASCADE_SRC_COMMON_SOCKSERVER_H

#include "common/sockstream.h"

namespace cascade {

// This class encapsulates the behavior of a TCP or UNIX domain socket server.

class sockserver {
  public:
    sockserver(uint32_t port, size_t backlog);
    sockserver(const char* path, size_t backlog);  
    ~sockserver();

    sockstream* accept();

    // Returns true if an error occurred while creating this socket
    bool error() const;
    // Returns true if the file descriptor underlying this socket is valid
    bool valid() const;
    // Returns the file descriptor underlying this socket
    int descriptor() const;

  private:
    int fd_; 
};

inline sockserver::sockserver(uint32_t port, size_t backlog) {
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ == -1) {
    return;
  } else if (::bind(fd_, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
    fd_ = -1;
  } else if (::listen(fd_, backlog) != 0) {
    fd_ = -1;
  } 
}

inline sockserver::sockserver(const char* path, size_t backlog) {
  struct sockaddr_un addr;
  bzero(&addr, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
  unlink(path);

  fd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd_ == -1) {
    return;
  } else if (::bind(fd_, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
    fd_ = -1;
  } else if (::listen(fd_, backlog) != 0) {
    fd_ = -1;
  } 
}

inline sockserver::~sockserver() {
  if (fd_ != -1) {
    ::close(fd_);
  }
}

inline sockstream* sockserver::accept() {
  return fd_ == -1 ? new sockstream(-1) : new sockstream(::accept(fd_, nullptr, nullptr));
}

inline bool sockserver::error() const {
  return fd_ == -1;
}

inline bool sockserver::valid() const {
  errno = 0;
  return (fcntl(fd_, F_GETFD) != -1) || (errno != EBADF);
}

inline int sockserver::descriptor() const {
  return fd_;
}

} // namespace cascade

#endif
