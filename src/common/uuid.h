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

#ifndef CASCADE_SRC_COMMON_UUID_H
#define CASCADE_SRC_COMMON_UUID_H

#include <iostream>
#include <uuid/uuid.h>
#include "common/serializable.h"

namespace cascade {

// This class is a simple wrapper around the DCE compatible Universally Unique
// Identifier Library. It can be used wherever objects need to be uniquely
// id'ed between different processes.

class Uuid : public Serializable {
  public:
    Uuid();
    Uuid(const Uuid& rhs);
    Uuid& operator=(const Uuid& rhs);
    ~Uuid() override = default;

    size_t deserialize(std::istream& is) override;
    size_t serialize(std::ostream& os) const override;

  private:
    uuid_t id_;
};

inline Uuid::Uuid() : Serializable() {
  uuid_generate(id_);
}

inline Uuid::Uuid(const Uuid& rhs) {
  uuid_copy(id_, rhs.id_);
}

inline Uuid& Uuid::operator=(const Uuid& rhs) {
  uuid_copy(id_, rhs.id_);
  return *this;
}

inline size_t Uuid::deserialize(std::istream& is) {
  is.read(reinterpret_cast<char*>(&id_), sizeof(id_));
  return sizeof(id_);
}

inline size_t Uuid::serialize(std::ostream& os) const {
  os.write(reinterpret_cast<const char*>(&id_), sizeof(id_));
  return sizeof(id_);
}

} // namespace cascade

#endif
