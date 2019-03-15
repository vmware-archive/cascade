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

#include "src/base/bits/bits.h"
#include "src/base/stream/bufstream.h"
#include "src/target/common/connection.h"
#include "src/target/common/rpc.h"
#include "src/target/common/sys_task.h"
#include "src/target/common/value.h"
#include "src/target/core.h"
#include "src/target/input.h"
#include "src/target/interface.h"
#include "src/target/state.h"

namespace cascade {

template <typename T>
class ProxyCore : public T {
  public:
    ProxyCore(Interface* interface, Rpc::Id id, Connection* conn);
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
    Rpc::Id id_;
    Connection* conn_;

    Bits bits_;
    bufstream in_buf_;
    bufstream out_buf_;

    void recv_response();
    void recv_task(const SysTask& t);
}; 

template <typename T>
inline ProxyCore<T>::ProxyCore(Interface* interface, Rpc::Id id, Connection* conn) : T(interface), in_buf_(256), out_buf_(256) {
  id_ = id;
  conn_ = conn;
}

template <typename T>
inline ProxyCore<T>::~ProxyCore() {
  conn_->send_rpc(Rpc(Rpc::Type::ENGINE_TEARDOWN, id_));
  conn_->recv_ack();
}

template <typename T>
inline State* ProxyCore<T>::get_state() {
  conn_->send_rpc(Rpc(Rpc::Type::GET_STATE, id_));
  conn_->recv_str(in_buf_);

  auto* s = new State();
  s->deserialize(in_buf_);
  in_buf_.clear();

  return s;
}

template <typename T>
inline void ProxyCore<T>::set_state(const State* s) {
  conn_->send_rpc(Rpc(Rpc::Type::SET_STATE, id_));
  s->serialize(out_buf_);
  conn_->send_str(out_buf_);
  conn_->recv_ack();
  out_buf_.resize(0);
}

template <typename T>
inline Input* ProxyCore<T>::get_input() {
  conn_->send_rpc(Rpc(Rpc::Type::GET_INPUT, id_));
  conn_->recv_str(in_buf_);

  auto* i = new Input();
  i->deserialize(in_buf_);
  in_buf_.clear();

  return i;
}

template <typename T>
inline void ProxyCore<T>::set_input(const Input* i) {
  conn_->send_rpc(Rpc(Rpc::Type::SET_INPUT, id_));
  i->serialize(out_buf_);
  conn_->send_str(out_buf_);
  conn_->recv_ack();
  out_buf_.resize(0);
}

template <typename T>
inline void ProxyCore<T>::finalize() {
  conn_->send_rpc(Rpc(Rpc::Type::FINALIZE, id_));
  conn_->recv_ack();
}

template <typename T>
inline bool ProxyCore<T>::overrides_done_step() const {
  conn_->send_rpc(Rpc(Rpc::Type::OVERRIDES_DONE_STEP, id_));
  auto res = false;
  conn_->recv_bool(res);
  return res;
}

template <typename T>
inline void ProxyCore<T>::done_step() {
  conn_->send_rpc(Rpc(Rpc::Type::DONE_STEP, id_));
  conn_->recv_ack();
}

template <typename T>
inline bool ProxyCore<T>::overrides_done_simulation() const {
  conn_->send_rpc(Rpc(Rpc::Type::OVERRIDES_DONE_SIMULATION, id_));
  auto res = false;
  conn_->recv_bool(res);
  return res;
}

template <typename T>
inline void ProxyCore<T>::done_simulation() {
  conn_->send_rpc(Rpc(Rpc::Type::DONE_SIMULATION, id_));
  conn_->recv_ack();
}

template <typename T>
inline void ProxyCore<T>::read(VId id, const Bits* b) {
  Value(id, const_cast<Bits*>(b)).serialize(out_buf_);
}

template <typename T>
inline void ProxyCore<T>::evaluate() {
  conn_->send_rpc(Rpc(Rpc::Type::EVALUATE, id_));
  conn_->send_str(out_buf_);
  out_buf_.resize(0);
  recv_response();
}

template <typename T>
inline bool ProxyCore<T>::there_are_updates() const {
  conn_->send_rpc(Rpc(Rpc::Type::THERE_ARE_UPDATES, id_));
  auto res = false;
  conn_->recv_bool(res);
  return res;
}

template <typename T>
inline void ProxyCore<T>::update() {
  conn_->send_rpc(Rpc(Rpc::Type::UPDATE, id_));
  conn_->send_str(out_buf_);
  out_buf_.resize(0);
  recv_response();
}

template <typename T>
inline bool ProxyCore<T>::there_were_tasks() const {
  conn_->send_rpc(Rpc(Rpc::Type::THERE_WERE_TASKS, id_));
  auto res = false;
  conn_->recv_bool(res);
  return res;
}

template <typename T>
inline bool ProxyCore<T>::conditional_update() {
  conn_->send_rpc(Rpc(Rpc::Type::CONDITIONAL_UPDATE, id_));
  conn_->send_str(out_buf_);
  out_buf_.resize(0);

  auto res = false;
  conn_->recv_bool(res);
  recv_response();
  return res;
}

template <typename T>
inline size_t ProxyCore<T>::open_loop(VId clk, bool val, size_t itr) {
  conn_->send_rpc(Rpc(Rpc::Type::OPEN_LOOP, id_));
  conn_->send_double(static_cast<uint32_t>(clk));
  conn_->send_bool(val);
  conn_->send_double(static_cast<uint32_t>(itr));
  uint32_t res = 0;
  conn_->recv_double(res);
  recv_response();
  return res;
}

template <typename T>
inline void ProxyCore<T>::recv_response() {
  conn_->recv_str(in_buf_);
  auto type = false;
  for (in_buf_.read(reinterpret_cast<char*>(&type), sizeof(type)); !in_buf_.eof(); in_buf_.read(reinterpret_cast<char*>(&type), sizeof(type))) {
    if (type) {
      Value temp(0, &bits_);
      temp.deserialize(in_buf_);
      T::interface()->write(temp.id_, temp.val_);
    } else {
      SysTask temp;
      temp.deserialize(in_buf_);
      recv_task(temp);
    }
  }
  in_buf_.clear();
}

template <typename T>
inline void ProxyCore<T>::recv_task(const SysTask& t) {
  switch (t.type_) {
    case SysTask::Type::DISPLAY:
      return T::interface()->display(t.text_);
    case SysTask::Type::ERROR:
      return T::interface()->error(t.text_);
    case SysTask::Type::FINISH:
      return T::interface()->finish(t.arg_);
    case SysTask::Type::INFO:
      return T::interface()->info(t.text_);
    case SysTask::Type::RESTART:
      return T::interface()->restart(t.text_);
    case SysTask::Type::RETARGET:
      return T::interface()->retarget(t.text_);
    case SysTask::Type::SAVE:
      return T::interface()->save(t.text_);
    case SysTask::Type::WARNING:
      return T::interface()->warning(t.text_);
    case SysTask::Type::WRITE:
      return T::interface()->write(t.text_);
    default:
      T::interface()->error("Unrecognized sys task rpc!");
      T::interface()->finish(0);
      return;
  }
}

} // namespace cascade

#endif
