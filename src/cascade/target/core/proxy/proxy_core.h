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

#ifndef CASCADE_SRC_TARGET_CORE_PROXY_PROXY_CORE_H
#define CASCADE_SRC_TARGET_CORE_PROXY_PROXY_CORE_H

#include <string>
#include "common/bits.h"
#include "common/sockstream.h"
#include "target/compiler/rpc.h"
#include "target/core.h"
#include "target/input.h"
#include "target/interface.h"
#include "target/state.h"

namespace cascade::proxy {

template <typename T>
class ProxyCore : public T {
  public:
    ProxyCore(Interface* interface, uint32_t pid, uint32_t eid, uint32_t n, sockstream* sock);
    ~ProxyCore() override;

    State* get_state() override;
    void set_state(const State* s) override;
    Input* get_input() override;
    void set_input(const Input* i) override;
    void finalize() override;

    bool overrides_done_step() const override;
    void done_step() override;
    bool overrides_done_simulation() const override;
    void done_simulation() override;

    void read(VId id, const Bits* b) override;
    void evaluate() override;
    bool there_are_updates() const override;
    void update() override;
    bool there_were_tasks() const override;

    bool conditional_update() override;
    size_t open_loop(VId clk, bool val, size_t itr) override;

  private:
    uint32_t pid_;
    uint32_t eid_;
    uint32_t n_;
    sockstream* sock_;

    void recv();
}; 

template <typename T>
inline ProxyCore<T>::ProxyCore(Interface* interface, uint32_t pid, uint32_t eid, uint32_t n, sockstream* sock) : T(interface) {
  pid_ = pid;
  eid_ = eid;
  n_ = n;
  sock_ = sock;
}

template <typename T>
inline ProxyCore<T>::~ProxyCore() {
  Rpc(Rpc::Type::TEARDOWN_ENGINE, pid_, eid_, n_).serialize(*sock_);
  sock_->flush();
  recv();
}

template <typename T>
inline State* ProxyCore<T>::get_state() {
  Rpc(Rpc::Type::GET_STATE, pid_, eid_, n_).serialize(*sock_);
  sock_->flush();

  auto* s = new State();
  s->deserialize(*sock_);
  return s;
}

template <typename T>
inline void ProxyCore<T>::set_state(const State* s) {
  Rpc(Rpc::Type::SET_STATE, pid_, eid_, n_).serialize(*sock_);
  s->serialize(*sock_);
  sock_->flush();
}

template <typename T>
inline Input* ProxyCore<T>::get_input() {
  Rpc(Rpc::Type::GET_INPUT, pid_, eid_, n_).serialize(*sock_);
  sock_->flush();

  auto* i = new Input();
  i->deserialize(*sock_);
  return i;
}

template <typename T>
inline void ProxyCore<T>::set_input(const Input* i) {
  Rpc(Rpc::Type::SET_INPUT, pid_, eid_, n_).serialize(*sock_);
  i->serialize(*sock_);
  sock_->flush();
}

template <typename T>
inline void ProxyCore<T>::finalize() {
  Rpc(Rpc::Type::FINALIZE, pid_, eid_, n_).serialize(*sock_);
  sock_->flush();
  recv();
}

template <typename T>
inline bool ProxyCore<T>::overrides_done_step() const {
  Rpc(Rpc::Type::OVERRIDES_DONE_STEP, pid_, eid_, n_).serialize(*sock_);
  sock_->flush();
  return (sock_->get() == 1);
}

template <typename T>
inline void ProxyCore<T>::done_step() {
  Rpc(Rpc::Type::DONE_STEP, pid_, eid_, n_).serialize(*sock_);
  sock_->flush();
}

template <typename T>
inline bool ProxyCore<T>::overrides_done_simulation() const {
  Rpc(Rpc::Type::OVERRIDES_DONE_SIMULATION, pid_, eid_, n_).serialize(*sock_);
  sock_->flush();
  return (sock_->get() == 1);
}

template <typename T>
inline void ProxyCore<T>::done_simulation() {
  Rpc(Rpc::Type::DONE_SIMULATION, pid_, eid_, n_).serialize(*sock_);
  sock_->flush();
}

template <typename T>
inline void ProxyCore<T>::read(VId id, const Bits* b) {
  Rpc(Rpc::Type::READ, pid_, eid_, n_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&id), 4);
  b->serialize(*sock_);

  // Don't flush. The only time these actually need to go out is before calling
  // evaluate(), update(), conditional_update(), or open_loop()
}

template <typename T>
inline void ProxyCore<T>::evaluate() {
  Rpc(Rpc::Type::EVALUATE, pid_, eid_, n_).serialize(*sock_);
  // This call to flush dumps any reads which have been enqueued
  sock_->flush();
  recv();
}

template <typename T>
inline bool ProxyCore<T>::there_are_updates() const {
  Rpc(Rpc::Type::THERE_ARE_UPDATES, pid_, eid_, n_).serialize(*sock_);
  sock_->flush();
  return (sock_->get() == 1);
}

template <typename T>
inline void ProxyCore<T>::update() {
  Rpc(Rpc::Type::UPDATE, pid_, eid_, n_).serialize(*sock_);
  // This call to flush dumps any reads which have been enqueued
  sock_->flush();
  recv();
}

template <typename T>
inline bool ProxyCore<T>::there_were_tasks() const {
  Rpc(Rpc::Type::THERE_WERE_TASKS, pid_, eid_, n_).serialize(*sock_);
  sock_->flush();
  return (sock_->get() == 1);
}

template <typename T>
inline bool ProxyCore<T>::conditional_update() {
  Rpc(Rpc::Type::CONDITIONAL_UPDATE, pid_, eid_, n_).serialize(*sock_);
  // This call to flush dumps any reads which have been enqueued
  sock_->flush();
  recv();
  return (sock_->get() == 1);
}

template <typename T>
inline size_t ProxyCore<T>::open_loop(VId clk, bool val, size_t itr) {
  Rpc(Rpc::Type::OPEN_LOOP, pid_, eid_, n_).serialize(*sock_);
  sock_->write(reinterpret_cast<const char*>(&clk), 4);
  sock_->put(val ? 1 : 0);
  sock_->write(reinterpret_cast<const char*>(&itr), 4);
  // This call to flush dumps any reads which have been enqueued
  sock_->flush();
  recv();
  uint32_t res = 0;
  sock_->read(reinterpret_cast<char*>(&res), 4);
  return res;
}

template <typename T>
inline void ProxyCore<T>::recv() {
  Rpc rpc;
  while (rpc.deserialize(*sock_)) {
    switch(rpc.type_) {
      case Rpc::Type::WRITE_BITS: {
        VId id = 0;
        Bits bits;
        sock_->read(reinterpret_cast<char*>(&id), 4);
        bits.deserialize(*sock_);
        T::interface()->write(id, &bits);
        break;
      }
      case Rpc::Type::WRITE_BOOL: {
        VId id = 0;
        bool b = false;
        sock_->read(reinterpret_cast<char*>(&id), 4);
        b = (sock_->get() == 1);
        T::interface()->write(id, b);
        break;
      }

      case Rpc::Type::DEBUG: {
        uint32_t action = 0;
        sock_->read(reinterpret_cast<char*>(&action), 4);
        std::string arg = "";
        getline(*sock_, arg, '\0');
        T::interface()->debug(action, arg);
        break;
      }
      case Rpc::Type::FINISH: {
        uint32_t code = 0;
        sock_->read(reinterpret_cast<char*>(&code), 4);
        T::interface()->finish(code);
        break;
      }
      case Rpc::Type::RESTART: {
        std::string path = "";
        getline(*sock_, path, '\0');
        T::interface()->restart(path);
        break;
      }
      case Rpc::Type::RETARGET: {
        std::string path = "";
        getline(*sock_, path, '\0');
        T::interface()->retarget(path);
        break;
      }
      case Rpc::Type::SAVE: {
        std::string path = "";
        getline(*sock_, path, '\0');
        T::interface()->save(path);
        break;
      }

      case Rpc::Type::FOPEN: {
        std::string path = "";
        uint8_t mode = 0;
        getline(*sock_, path, '\0');
        sock_->read(reinterpret_cast<char*>(&mode), sizeof(mode));

        FId res = T::interface()->fopen(path, mode);
        sock_->write(reinterpret_cast<char*>(&res), sizeof(res));
        sock_->flush();
        break;
      }
      case Rpc::Type::IN_AVAIL: {
        FId id = 0;
        sock_->read(reinterpret_cast<char*>(&id), sizeof(id));

        int32_t res = T::interface()->in_avail(id);
        sock_->write(reinterpret_cast<char*>(&res), sizeof(res));
        sock_->flush();
        break;
      }
      case Rpc::Type::PUBSEEKOFF: {
        FId id = 0;
        int32_t off = 0;
        uint8_t way = 0;
        uint8_t which = 0;
        sock_->read(reinterpret_cast<char*>(&id), sizeof(id));
        sock_->read(reinterpret_cast<char*>(&off), sizeof(off));
        sock_->read(reinterpret_cast<char*>(&way), sizeof(way));
        sock_->read(reinterpret_cast<char*>(&which), sizeof(which));

        uint32_t res = T::interface()->pubseekoff(id, off, way, which);
        sock_->write(reinterpret_cast<char*>(&res), sizeof(res));
        sock_->flush();
        break;
      }
      case Rpc::Type::PUBSEEKPOS: {
        FId id = 0;
        int32_t pos = 0;
        uint8_t which = 0;
        sock_->read(reinterpret_cast<char*>(&id), sizeof(id));
        sock_->read(reinterpret_cast<char*>(&pos), sizeof(pos));
        sock_->read(reinterpret_cast<char*>(&which), sizeof(which));

        uint32_t res = T::interface()->pubseekpos(id, pos, which);
        sock_->write(reinterpret_cast<char*>(&res), sizeof(res));
        sock_->flush();
        break;
      }
      case Rpc::Type::PUBSYNC: {
        FId id = 0;
        sock_->read(reinterpret_cast<char*>(&id), sizeof(id));

        int32_t res = T::interface()->pubsync(id);
        sock_->write(reinterpret_cast<char*>(&res), sizeof(res));
        sock_->flush();
        break;
      }
      case Rpc::Type::SBUMPC: {
        FId id = 0;
        sock_->read(reinterpret_cast<char*>(&id), sizeof(id));

        int32_t res = T::interface()->sbumpc(id);
        sock_->write(reinterpret_cast<char*>(&res), sizeof(res));
        sock_->flush();
        break;
      }
      case Rpc::Type::SGETC: {
        FId id = 0;
        sock_->read(reinterpret_cast<char*>(&id), sizeof(id));

        int32_t res = T::interface()->sgetc(id);
        sock_->write(reinterpret_cast<char*>(&res), sizeof(res));
        sock_->flush();
        break;
      }
      case Rpc::Type::SGETN: {
        FId id = 0;
        uint32_t n = 0;
        sock_->read(reinterpret_cast<char*>(&id), sizeof(id));
        sock_->read(reinterpret_cast<char*>(&n), sizeof(n));

        auto* c = new char[n];
        uint32_t res = T::interface()->sgetn(id, c, n);
        sock_->write(reinterpret_cast<char*>(&res), sizeof(res));
        sock_->write(c, res);
        sock_->flush();
        delete[] c;
        break;
      }
      case Rpc::Type::SPUTC: {
        FId id = 0;
        sock_->read(reinterpret_cast<char*>(&id), sizeof(id));
        auto c = sock_->get();

        int32_t res = T::interface()->sputc(id, c);
        sock_->write(reinterpret_cast<char*>(&res), sizeof(res));
        sock_->flush();
        break;
      }
      case Rpc::Type::SPUTN: {
        FId id = 0;
        uint32_t n = 0;
        sock_->read(reinterpret_cast<char*>(&id), sizeof(id));
        sock_->read(reinterpret_cast<char*>(&n), sizeof(n));
        auto* c = new char[n];
        sock_->read(c, n);

        uint32_t res = T::interface()->sputn(id, c, n);
        sock_->write(reinterpret_cast<char*>(&res), sizeof(res));
        sock_->flush();
        delete[] c;
        break;
      }

      case Rpc::Type::OKAY:
      default:
        return;
    }
  }
}

} // namespace cascade::proxy

#endif
