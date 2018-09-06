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

#ifndef CASCADE_SRC_VERILOG_ANALYZE_INDICES_H
#define CASCADE_SRC_VERILOG_ANALYZE_INDICES_H

#include <stddef.h>
#include "src/verilog/ast/ast_fwd.h"

namespace cascade {

// A collection of methods for hashing identifiers based on their names rather
// than their pointer value (eg hash(x) or hash(x[i+1]). These methods require
// up-to-date resolution decorations (see resolve.h) to function correctly.
// These methods are undefined for identifiers whos free variables are not
// compile-time constants.
struct HashId {
  size_t operator()(const Maybe<Expression>* e) const;
  size_t operator()(const Id* id) const;
  size_t operator()(const Identifier* id) const;
};

// A collection of methods for checking equality of identifiers based on their
// names rather than their pointer value (eg x[i+1] =?= x[2]) These methods
// require up-to-date resolution decorations (see resolve.h) to function
// correctly.  These methods are undefined for identifiers whos free variables
// are not compile-time constants.
struct EqId {
  bool operator()(const Maybe<Expression>* e1, const Maybe<Expression>* e2) const;
  bool operator()(const Id* id1, const Id* id2) const;
  bool operator()(const Identifier* id1, const Identifier* id2) const;
};

// A collection of methods for checking equality of identifiers based on their
// names rather than their pointer value (eg x[i+1] =?= x[3]) These methods
// require up-to-date resolution decorations (see resolve.h) to function
// correctly.  These methods are undefined for identifiers whos free variables
// are not compile-time constants.
struct LtId {
  bool operator()(const Maybe<Expression>* e1, const Maybe<Expression>* e2) const;
  bool operator()(const Id* id1, const Id* id2) const;
  bool operator()(const Identifier* id1, const Identifier* id2) const;
};

} // namespace cascade

#endif
