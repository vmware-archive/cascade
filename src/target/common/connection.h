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

#ifndef CASCADE_SRC_TARGET_COMMON_CONNECTION_H
#define CASCADE_SRC_TARGET_COMMON_CONNECTION_H

#include "src/base/socket/socket.h"
#include "src/base/stream/bufstream.h"
#include "src/target/common/rpc.h"

namespace cascade {

class Connection {
  public:
    Connection(Socket* sock);
    ~Connection();

    bool error() const;
    int descriptor() const;

    void recv_ack();
    void send_ack();
    void recv_rpc(Rpc& rpc);    
    void send_rpc(const Rpc& rpc);
    void recv_str(bufstream& bs);
    void send_str(const bufstream& bs);
    void recv_bool(bool& b);
    void send_bool(bool b);
    void recv_double(uint32_t& d);
    void send_double(uint32_t d);

  private:
    Socket* sock_;
};

inline Connection::Connection(Socket* sock) {
  sock_ = sock;
}

inline Connection::~Connection() {
  if (sock_ != nullptr) {
    delete sock_;
  }
}

inline bool Connection::error() const {
  return sock_->error();
}

inline int Connection::descriptor() const {
  return sock_->descriptor();
} 

inline void Connection::recv_ack() {
  bool ignore;
  sock_->recv(ignore);
}

inline void Connection::send_ack() {
  sock_->send(true);
}

inline void Connection::recv_rpc(Rpc& rpc) {
  sock_->recv((char*)&rpc.type_, sizeof(rpc.type_));
  sock_->recv((char*)&rpc.id_, sizeof(rpc.id_));
}

inline void Connection::send_rpc(const Rpc& rpc) {
  sock_->send((char*)&rpc.type_, sizeof(rpc.type_));
  sock_->send((char*)&rpc.id_, sizeof(rpc.id_));
}

inline void Connection::recv_str(bufstream& bs) {
  uint32_t len = 0;
  sock_->recv((char*)&len, sizeof(len));
  bs.resize(len);
  sock_->recv((char*)bs.data(), len);
}

inline void Connection::send_str(const bufstream& bs) {
  const uint32_t len = bs.size();
  sock_->send((char*)&len, sizeof(len));
  sock_->send((char*)bs.data(), len);
}

inline void Connection::recv_bool(bool& b) {
  sock_->recv<bool>(b);
}

inline void Connection::send_bool(bool b) {
  sock_->send<bool>(b);
}

inline void Connection::recv_double(uint32_t& d) {
  sock_->recv<uint32_t>(d);
}

inline void Connection::send_double(uint32_t d) {
  sock_->send<uint32_t>(d);
}

} // namespace cascade 

#endif
