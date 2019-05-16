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

#ifndef CASCADE_SRC_CL_GROUP_H
#define CASCADE_SRC_CL_GROUP_H

#include <algorithm>
#include "arg_table.h"
#include "singleton.h"

namespace cascade::cl {

class Group {
  public:
    static Group& create(const std::string& name) {
      return *(new Group(name));
    }

    Group(const Group& rhs) = delete;
    Group(const Group&& rhs) = delete;
    Group& operator=(Group& rhs) = delete;
    Group& operator=(Group&& rhs) = delete;

    const std::string& name() const {
      return name_;
    }
    typedef std::vector<Arg*>::const_iterator arg_itr;
    arg_itr arg_begin() const {
      return Singleton<ArgTable>::get().args_by_group_[idx_].begin();
    }
    arg_itr arg_end() const {
      return Singleton<ArgTable>::get().args_by_group_[idx_].end();
    }

  private:
    Group(const std::string& name) : name_(name) { 
      auto& table = Singleton<ArgTable>::get();
      table.groups_.push_back(this);
      idx_ = table.args_by_group_.size();
      table.args_by_group_.resize(idx_+1);
    }

    std::string name_;
    size_t idx_;
};

} // namespace cascade::cl

#endif
