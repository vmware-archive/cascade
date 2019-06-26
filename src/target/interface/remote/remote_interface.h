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

#ifndef CASCADE_SRC_TARGET_INTERFACE_REMOTE_REMOTE_INTERFACE_H
#define CASCADE_SRC_TARGET_INTERFACE_REMOTE_REMOTE_INTERFACE_H

#include <cassert>
#include "common/sockstream.h"
#include "target/common/rpc.h"
#include "target/interface.h"

namespace cascade {

class RemoteInterface : public Interface {
  public:
    explicit RemoteInterface(sockstream* sock, Rpc::Id id);
    ~RemoteInterface() override = default;

    void write(VId id, const Bits* b) override;
    void write(VId id, bool b) override;

    void finish(uint32_t arg) override;
    void restart(const std::string& s) override;
    void retarget(const std::string& s) override;
    void save(const std::string& s) override;

    FId fopen(const std::string& path) override;
    int32_t in_avail(FId id) override;
    uint32_t pubseekoff(FId id, int32_t off, uint8_t way, uint8_t which) override;
    uint32_t pubseekpos(FId id, int32_t pos, uint8_t which) override;
    int32_t pubsync(FId id) override;
    int32_t sbumpc(FId id) override;
    int32_t sgetc(FId id) override;
    uint32_t sgetn(FId id, char* c, uint32_t n) override;
    int32_t sputc(FId id, char c) override;
    uint32_t sputn(FId id, const char* c, uint32_t n) override;
      
  private:
    sockstream* sock_;
    Rpc::Id id_;
}; 

inline RemoteInterface::RemoteInterface(sockstream* sock, Rpc::Id id) : Interface() {
  sock_ = sock;
  id_ = id;
}

inline void RemoteInterface::write(VId id, const Bits* b) {
  Rpc(Rpc::Type::WRITE_BITS, id_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&id), sizeof(id));
  b->serialize(*sock_);
}

inline void RemoteInterface::write(VId id, bool b) {
  Rpc(Rpc::Type::WRITE_BOOL, id_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&id), sizeof(id));
  sock_->put(b ? 1 : 0);
}

inline void RemoteInterface::finish(uint32_t arg) {
  Rpc(Rpc::Type::FINISH, id_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&arg), 4);
}

inline void RemoteInterface::restart(const std::string& s) {
  Rpc(Rpc::Type::RESTART, id_).serialize(*sock_);
  sock_->write(s.c_str(), s.length());
  sock_->put('\0');
}

inline void RemoteInterface::retarget(const std::string& s) {
  Rpc(Rpc::Type::RETARGET, id_).serialize(*sock_);
  sock_->write(s.c_str(), s.length());
  sock_->put('\0');
}

inline void RemoteInterface::save(const std::string& s) {
  Rpc(Rpc::Type::SAVE, id_).serialize(*sock_);
  sock_->write(s.c_str(), s.length());
  sock_->put('\0');
}

inline FId RemoteInterface::fopen(const std::string& path) {
  Rpc(Rpc::Type::FOPEN, id_).serialize(*sock_);
  sock_->write(path.c_str(), path.length());
  sock_->put('\0');
  sock_->flush();

  FId res;
  sock_->read(reinterpret_cast<char*>(&res), sizeof(FId));
  return res;
}

inline int32_t RemoteInterface::in_avail(FId id) {
  Rpc(Rpc::Type::IN_AVAIL, id_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&id), sizeof(id));
  sock_->flush();

  int32_t res;
  sock_->read(reinterpret_cast<char*>(&res), sizeof(res));
  return res;
}

inline uint32_t RemoteInterface::pubseekoff(FId id, int32_t off, uint8_t way, uint8_t which) {
  Rpc(Rpc::Type::PUBSEEKOFF, id_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&id), sizeof(id));
  sock_->write(reinterpret_cast<const char*>(&off), sizeof(off));
  sock_->write(reinterpret_cast<const char*>(&way), sizeof(way));
  sock_->write(reinterpret_cast<const char*>(&which), sizeof(which));
  sock_->flush();

  uint32_t res;
  sock_->read(reinterpret_cast<char*>(&res), sizeof(res));
  return res;
}

inline uint32_t RemoteInterface::pubseekpos(FId id, int32_t pos, uint8_t which) {
  Rpc(Rpc::Type::PUBSEEKPOS, id_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&id), sizeof(id));
  sock_->write(reinterpret_cast<const char*>(&pos), sizeof(pos));
  sock_->write(reinterpret_cast<const char*>(&which), sizeof(which));
  sock_->flush();

  uint32_t res;
  sock_->read(reinterpret_cast<char*>(&res), sizeof(res));
  return res;
}

inline int32_t RemoteInterface::pubsync(FId id) {
  Rpc(Rpc::Type::PUBSYNC, id_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&id), sizeof(id));
  sock_->flush();

  int32_t res;
  sock_->read(reinterpret_cast<char*>(&res), sizeof(res));
  return res;
}

inline int32_t RemoteInterface::sbumpc(FId id) {
  Rpc(Rpc::Type::SBUMPC, id_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&id), sizeof(id));
  sock_->flush();

  int32_t res;
  sock_->read(reinterpret_cast<char*>(&res), sizeof(res));
  return res;
}

inline int32_t RemoteInterface::sgetc(FId id) {
  Rpc(Rpc::Type::SGETC, id_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&id), sizeof(id));
  sock_->flush();

  int32_t res;
  sock_->read(reinterpret_cast<char*>(&res), sizeof(res));
  return res;
}

inline uint32_t RemoteInterface::sgetn(FId id, char* c, uint32_t n) {
  Rpc(Rpc::Type::SGETN, id_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&id), sizeof(id));
  sock_->write(reinterpret_cast<const char*>(&n), sizeof(n));
  sock_->flush();

  uint32_t res;
  sock_->read(reinterpret_cast<char*>(&res), sizeof(res));
  sock_->read(c, res);
  return res;
}

inline int32_t RemoteInterface::sputc(FId id, char c) {
  Rpc(Rpc::Type::SPUTC, id_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&id), sizeof(id));
  sock_->put(c);
  sock_->flush();

  int32_t res;
  sock_->read(reinterpret_cast<char*>(&res), sizeof(res));
  return res;
}

inline uint32_t RemoteInterface::sputn(FId id, const char* c, uint32_t n) {
  Rpc(Rpc::Type::SPUTN, id_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&id), sizeof(id));
  sock_->write(reinterpret_cast<const char*>(&n), sizeof(n));
  sock_->write(c, n);
  sock_->flush();

  uint32_t res;
  sock_->read(reinterpret_cast<char*>(&res), sizeof(res));
  return res;
}

} // namespace cascade

#endif
