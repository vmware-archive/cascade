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

#ifndef CASCADE_SRC_CL_STR_ARG_H
#define CASCADE_SRC_CL_STR_ARG_H

#include "arg.h"

namespace cascade::cl {

template <typename T>
struct StrReader {
  bool operator()(std::istream& is, T& t) const {
    is >> t;
    return !is.fail();
  }
};  
template <typename T>
struct StrWriter {
  void operator()(std::ostream& os, const T& t) const {
    os << t;
  }
};

template <typename T, typename R = StrReader<T>, typename W = StrWriter<T>, size_t Arity = 1>
class StrArg : public Arg {
  public:
    static StrArg& create(const std::string& name) {
      return *(new StrArg(name));
    }

    StrArg& alias(const std::string& a) {
      names_.insert(a);
      return *this;
    }
    StrArg& description(const std::string& d) {
      desc_ = d;
      return *this;
    }
    StrArg& usage(const std::string& u) {
      usage_ = u;
      return *this;
    }
    StrArg& required() {
      req_ = true;
      return *this;
    }
    StrArg& initial(const T& val) {
      val_ = val;
      return *this;
    }
    const T& value() const {
      return val_;
    }
    operator const T&() const {
      return val_;
    }

    void read(std::istream& is) override {
      err_ = !R()(is, val_);
      dup_ = prov_;
      prov_ = true;
    }
    void write(std::ostream& os) const override {
      W()(os, val_);
    }
    size_t arity() const override {
      return Arity;
    }

  private:
    StrArg(const std::string& name) : Arg(name), val_() {}
    T val_;
};

} // namespace cascade::cl

#endif
