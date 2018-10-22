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

#ifndef CASCADE_SRC_BASE_SOCKET_H
#define CASCADE_SRC_BASE_SOCKET_H

#include <arpa/inet.h>
#include <cstring>
#include <resolv.h>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace cascade {

// This class is used to represent the concept of a socket. Depending on which
// constructor is chosen, it can be implemented in terms of either the TCP or
// UNIX Domain protocol.

// XXX: The use of this class is deprecated. See src/base/stream/sockstream.h for
// a better alternative.

class Socket {
  public:
    // Creates a socket in an unconnected state
    Socket();
    // Creates a socket from an already open file descriptor
    Socket(int fd);
    // Generic Teardown
    ~Socket();

    // Connect to a UNIX domain file descriptor
    void connect(const std::string& path);
    // Attach to a TCP file descriptor
    void connect(const std::string& host, uint32_t port);

    // Listen on a local port
    void listen(uint32_t port, size_t backlog);
    // Listen on a UNIX domain file descrptor
    void listen(const std::string& path, size_t backlog);

    // Inspectors:
    bool error() const;
    int descriptor() const;

    // Send/Receive
    void send(const char* c, size_t len);
    void recv(char* c, size_t len);

    template <typename T>
    void send(T t);
    template <typename T>
    void recv(T& t);

  private:
    int fd_;
    bool error_;
};

inline Socket::Socket() {
  fd_ = -1;
  error_ = true;
}

inline Socket::Socket(int fd) {
  fd_ = fd;
  error_ = false;
}

inline Socket::~Socket() {
  if (fd_ != -1) {
    ::close(fd_);
  }
}

inline void Socket::connect(const std::string& path) {
  struct sockaddr_un dest;
  bzero(&dest, sizeof(dest));
  dest.sun_family = AF_UNIX;
  strncpy(dest.sun_path, path.c_str(), sizeof(dest.sun_path)-1);

  fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
  error_ = (fd_ == -1) || (::connect(fd_, (struct sockaddr*)&dest, sizeof(dest)) != 0);
}

inline void Socket::connect(const std::string& host, uint32_t port) {
  struct sockaddr_in dest;
  bzero(&dest, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(port);
  inet_aton(host.c_str(), (in_addr*) &dest.sin_addr.s_addr);

  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  error_ = (fd_ == -1) || (::connect(fd_, (struct sockaddr*)&dest, sizeof(dest)) != 0);
}

inline void Socket::listen(uint32_t port, size_t backlog) {
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
  error_ = (fd_ == -1) || (::bind(fd_, (struct sockaddr*) &addr, sizeof(addr)) != 0) || (::listen(fd_, backlog) != 0);
}

inline void Socket::listen(const std::string& path, size_t backlog) {
  struct sockaddr_un addr;
  bzero(&addr, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path)-1);
  unlink(path.c_str());

  fd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
  error_ = (fd_ == -1) || (::bind(fd_, (struct sockaddr*) &addr, sizeof(addr)) != 0) || (::listen(fd_, backlog) != 0);
}

inline bool Socket::error() const {
  return error_;
}

inline int Socket::descriptor() const {
  return fd_;
} 

inline void Socket::send(const char* c, size_t len) {
  int total = 0;
  while (total < (int)len) {
    const auto res = ::send(fd_, c+total, len-total, 0);
    if (res == -1) {
      return;
    }
    total += res;
  }
}

inline void Socket::recv(char* c, size_t len) {
  int total = 0;
  while (total < (int)len) {
    const auto res = ::recv(fd_, c+total, len-total, 0);
    if (res == -1) {
      return;
    }
    total += res;
  }
}

template <typename T>
inline void Socket::send(T t) {
  send((char*)&t, sizeof(t));
}

template <typename T>
inline void Socket::recv(T& t) {
  recv((char*)&t, sizeof(t));
}

} // namespace cascade

#endif
