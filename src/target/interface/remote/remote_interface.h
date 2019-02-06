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
#include "src/target/common/sys_task.h"
#include "src/target/common/value.h"
#include "src/target/interface.h"

namespace cascade {

class RemoteInterface : public Interface {
  public:
    explicit RemoteInterface(bufstream* buf);
    ~RemoteInterface() override = default;

    void display(const std::string& s) override;
    void error(const std::string& s) override;
    void fatal(int arg, const std::string& s) override;
    void finish(int arg) override;
    void info(const std::string& s) override;
    void retarget(const std::string& s) override;
    void warning(const std::string& s) override;
    void write(const std::string& s) override;

    void write(VId id, const Bits* b) override;
      
  private:
    bufstream* buf_;

    void write_flag(bool flag);
}; 

inline RemoteInterface::RemoteInterface(bufstream* buf) : Interface() {
  buf_ = buf;
}

inline void RemoteInterface::display(const std::string& s) {
  write_flag(false);
  SysTask(SysTask::Type::DISPLAY, s).serialize(*buf_);
}

inline void RemoteInterface::error(const std::string& s) {
  write_flag(false);
  SysTask(SysTask::Type::ERROR, s).serialize(*buf_);
}

inline void RemoteInterface::fatal(int arg, const std::string& s) {
  write_flag(false);
  SysTask(SysTask::Type::FATAL, s, arg).serialize(*buf_);
}

inline void RemoteInterface::finish(int arg) {
  write_flag(false);
  SysTask(SysTask::Type::FINISH, "", arg).serialize(*buf_);
}

inline void RemoteInterface::info(const std::string& s) {
  write_flag(false);
  SysTask(SysTask::Type::INFO, s).serialize(*buf_);
}

inline void RemoteInterface::retarget(const std::string& s) {
  write_flag(false);
  SysTask(SysTask::Type::RETARGET, s).serialize(*buf_);
}

inline void RemoteInterface::warning(const std::string& s) {
  write_flag(false);
  SysTask(SysTask::Type::WARNING, s).serialize(*buf_);
}

inline void RemoteInterface::write(const std::string& s) {
  write_flag(false);
  SysTask(SysTask::Type::WRITE, s).serialize(*buf_);
}

inline void RemoteInterface::write(VId id, const Bits* b) {
  write_flag(true);
  Value(id, const_cast<Bits*>(b)).serialize(*buf_);
}

inline void RemoteInterface::write_flag(bool flag) {
  buf_->write(reinterpret_cast<char*>(&flag), sizeof(flag));
}

} // namespace cascade

#endif
