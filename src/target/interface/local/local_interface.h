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

#ifndef CASCADE_SRC_TARGET_INTERFACE_LOCAL_LOCAL_INTERFACE_H
#define CASCADE_SRC_TARGET_INTERFACE_LOCAL_LOCAL_INTERFACE_H

#include "src/runtime/data_plane.h"
#include "src/runtime/runtime.h"
#include "src/target/interface.h"

namespace cascade {

class LocalInterface : public Interface {
  public:
    LocalInterface(Runtime* rt);
    ~LocalInterface() override = default;     

    void display(const std::string& s) override;
    void write(const std::string& s) override;
    void finish(int arg) override;
    void error(const std::string& s) override;
    void warning(const std::string& s) override;
    void info(const std::string& s) override;
    void fatal(int arg, const std::string& s) override;

    void write(VId id, const Bits* b) override;
    void write(VId id, bool b) override;

  private:
    Runtime* rt_;
}; 

inline LocalInterface::LocalInterface(Runtime* rt) : Interface() {
  rt_ = rt;
}

inline void LocalInterface::display(const std::string& s) {
  rt_->display(s);
}

inline void LocalInterface::write(const std::string& s) {
  rt_->write(s);
}

inline void LocalInterface::finish(int arg) {
  rt_->finish(arg);
}

inline void LocalInterface::error(const std::string& s) {
  rt_->error(s);
}

inline void LocalInterface::warning(const std::string& s) {
  rt_->warning(s);
}

inline void LocalInterface::info(const std::string& s) {
  rt_->info(s);
}

inline void LocalInterface::fatal(int arg, const std::string& s) {
  rt_->fatal(arg, s);
}

inline void LocalInterface::write(VId id, const Bits* b) {
  rt_->write(id, b);
}

inline void LocalInterface::write(VId id, bool b) {
  rt_->write(id, b);
}

} // namespace cascade

#endif
