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

#ifndef CASCADE_SRC_TARGET_CORE_AVMM_VERILATOR_VERILATOR_LOGIC_H
#define CASCADE_SRC_TARGET_CORE_AVMM_VERILATOR_VERILATOR_LOGIC_H

#include "target/core/avmm/avmm_logic.h"
#include "target/core/avmm/verilator/verilator_compiler.h"
#include "target/core/avmm/verilator/verilator_logic.h"

namespace cascade::avmm {

template <size_t V, typename A, typename T>
class VerilatorLogic : public AvmmLogic<V,A,T> {
  public:
    VerilatorLogic(Interface* interface, ModuleDeclaration* md, size_t slot);
    virtual ~VerilatorLogic() override = default;

    void set_io(T(*read)(A), void(*write)(A,T)); 
};

template <size_t V, typename A, typename T>
inline VerilatorLogic<V,A,T>::VerilatorLogic(Interface* interface, ModuleDeclaration* md, size_t slot) : AvmmLogic<V,A,T>(interface, md, slot) { }

template <size_t V, typename A, typename T>
inline void VerilatorLogic<V,A,T>::set_io(T(*read)(A), void(write)(A,T)) {
  AvmmLogic<V,A,T>::get_table()->set_read([read](A index) {
    return read(index);
  });
  AvmmLogic<V,A,T>::get_table()->set_write([write](A index, T val) {
    write(index, val);
  });
}

} // namespace cascade::avmm

#endif
