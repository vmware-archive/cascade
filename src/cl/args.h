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

#ifndef CASCADE_SRC_CL_ARGS_H
#define CASCADE_SRC_CL_ARGS_H

#include <sstream>
#include "group.h"
#include "singleton.h"

namespace cascade::cl {

struct Args {
  static void read(int argc, char** argv) {
    for (int i = 0; i < argc; ++i) {
      const auto a = arg_find(argv[i]);
      if (a == arg_end()) {
        Singleton<ArgTable>::get().unrec_.push_back(argv[i]);
      } else {
        std::stringstream ss;
        ss << (((*a)->arity() == 1 && ++i < argc) ? argv[i] : "");
        (*a)->read(ss);
      }
    }
  }

  typedef std::vector<Group*>::iterator group_itr;
  static group_itr group_begin() { 
    return Singleton<ArgTable>::get().groups_.begin(); 
  } 
  static group_itr group_end() { 
    return Singleton<ArgTable>::get().groups_.end(); 
  } 
  static group_itr group_find(const std::string& name) {    
    return std::find_if(group_begin(), group_end(), [name](Group* g){return g->name() == name;});    
  }

  typedef std::vector<Arg*>::iterator arg_itr;
  static arg_itr arg_begin() { 
    return Singleton<ArgTable>::get().args_.begin(); 
  } 
  static arg_itr arg_end() { 
    return Singleton<ArgTable>::get().args_.end(); 
  } 
  static arg_itr arg_find(const std::string& alias) {   
    return std::find_if(arg_begin(), arg_end(), [alias](Arg* a){return a->matches(alias);});    
  }

  typedef std::vector<const char*>::iterator unrecognized_itr;
  static unrecognized_itr unrecognized_begin() { 
    return Singleton<ArgTable>::get().unrec_.begin(); 
  } 
  static unrecognized_itr unrecognized_end() { 
    return Singleton<ArgTable>::get().unrec_.end(); 
  } 
};

} // namespace cascade::cl

#endif
