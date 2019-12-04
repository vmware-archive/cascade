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

#ifndef CASCADE_SRC_CL_ARG_H
#define CASCADE_SRC_CL_ARG_H

#include <iostream>
#include <set>
#include <string>
#include "arg_table.h"
#include "group.h"
#include "singleton.h"

namespace cascade::cl {

class Arg {
  public:
    Arg(const std::string& name) : names_({{name}}), desc_(""), usage_(""), req_(false), prov_(false), dup_(false), err_(false) {
      auto& table = Singleton<ArgTable>::get();
      if (table.groups_.empty()) {
        Group::create("Ungrouped Arguments");
      }
      table.args_by_group_.back().push_back(this);
      table.args_.push_back(this);
    }
    Arg(const Arg& rhs) = delete;
    Arg(const Arg&& rhs) = delete;
    Arg& operator=(Arg& rhs) = delete;
    Arg& operator=(Arg&& rhs) = delete;
    virtual ~Arg() = default;

    typedef std::set<std::string>::const_iterator alias_itr;
    const alias_itr alias_begin() const {
      return names_.begin();
    }
    const alias_itr alias_end() const {
      return names_.end();
    }
    bool matches(const std::string& alias) const {    
      return names_.find(alias) != names_.end();    
    }
    const std::string& description() const {
      return desc_;
    }
    const std::string& usage() const {
      return usage_;
    }
    bool required() const {
      return req_;
    }
    bool provided() const {
      return prov_;
    }
    bool duplicated() const {
      return dup_;
    }
    bool error() const {
      return err_;
    }

    virtual void read(std::istream& is) = 0;
    virtual void write(std::ostream& os) const = 0;
    virtual size_t arity() const = 0;

  protected:
    std::set<std::string> names_;
    std::string desc_;
    std::string usage_;
    bool req_;
    bool prov_;
    bool dup_;
    bool err_;
};

} // namespace cascade::cl

#endif
