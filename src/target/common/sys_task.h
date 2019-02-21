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

#ifndef CASCADE_SRC_TARGET_COMMON_SYS_TASK_H
#define CASCADE_SRC_TARGET_COMMON_SYS_TASK_H

#include <iostream>
#include <string>
#include "src/base/serial/serializable.h"

namespace cascade {

struct SysTask : Serializable {
  enum class Type : uint8_t {
    DISPLAY = 0,
    ERROR,
    FATAL,
    FINISH,
    INFO,
    RESTART,
    RETARGET,
    SAVE,
    WARNING,
    WRITE
  };

  SysTask();
  SysTask(Type type, const std::string& text, uint8_t arg = 0);
  ~SysTask() override = default;

  size_t deserialize(std::istream& is) override;
  size_t serialize(std::ostream& os) const override;

  Type type_;
  std::string text_;
  uint8_t arg_;
};

inline SysTask::SysTask() : SysTask(Type::FATAL, "", 0) { }

inline SysTask::SysTask(Type type, const std::string& text, uint8_t arg) : Serializable() {
  type_ = type;
  text_ = text;
  arg_ = arg;
} 

inline size_t SysTask::deserialize(std::istream& is) {
  uint16_t len = 0;
  is.read(reinterpret_cast<char*>(&type_), sizeof(type_));
  is.read(reinterpret_cast<char*>(&len), sizeof(len));
  text_.resize(len);
  is.read(const_cast<char*>(reinterpret_cast<const char*>(text_.data())), len);
  is.read(reinterpret_cast<char*>(&arg_), sizeof(arg_));

  return sizeof(type_) + sizeof(len) + len + sizeof(arg_);
}

inline size_t SysTask::serialize(std::ostream& os) const {
  os.write(const_cast<char*>(reinterpret_cast<const char*>(&type_)), sizeof(type_));
  uint16_t len = text_.length();
  os.write(const_cast<char*>(reinterpret_cast<const char*>(&len)), sizeof(len));
  os.write(const_cast<char*>(reinterpret_cast<const char*>(text_.data())), len);
  os.write(const_cast<char*>(reinterpret_cast<const char*>(&arg_)), sizeof(arg_));

  return sizeof(type_) + sizeof(len) + len + sizeof(arg_);
}

} // namespace cascade

#endif
