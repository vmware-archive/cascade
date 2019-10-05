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

#ifndef CASCADE_SRC_VERILOG_BUILD_AST_BUILDER_H
#define CASCADE_SRC_VERILOG_BUILD_AST_BUILDER_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace cascade {

class Node;

class AstBuilder : public std::ostream {
  public:
    typedef std::vector<Node*>::iterator iterator;

    AstBuilder();
    AstBuilder(const std::string& s);
    ~AstBuilder() override = default;

    // Discards all previous results and parses everything which has been
    // passed to this stream since the last invocation of begin(). Ownership of
    // any resulting objects passes on to the caller.  On error, begin == end.
    iterator begin();
    // Returns a pointer to the end of the result range.
    iterator end();

    // Invokes begin, and returns the first element or nullptr if none exist.
    // All other elements are deleted.
    operator Node*();

  private:
    std::stringbuf sb_;
    std::vector<Node*> res_;
};

} // namespace caascade

#endif
