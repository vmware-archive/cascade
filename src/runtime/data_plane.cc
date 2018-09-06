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

#include "src/runtime/data_plane.h"

#include <algorithm>
#include <cassert>
#include "src/runtime/runtime.h"
#include "src/target/engine.h"

using namespace std;

namespace cascade {

DataPlane::DataPlane() { }

void DataPlane::register_id(const VId id) {
  if (id >= readers_.size()) {
    readers_.resize(id+1);
  }
  if (id >= writers_.size()) {
    writers_.resize(id+1);
  }
  if (id >= write_buf_.size()) {
    write_buf_.resize(id+1); 
  }
}

void DataPlane::register_reader(Engine* e, VId id) {
  assert(id < readers_.size());
  if (reader_find(e, id) == reader_end(id)) {
    readers_[id].push_back(e);
  }
}

void DataPlane::unregister_reader(Engine* e, VId id) {
  assert(id < readers_.size());
  if (reader_find(e, id) != reader_end(id)) {
    readers_[id].erase(reader_find(e, id));
  }
}

DataPlane::reader_iterator DataPlane::reader_find(Engine* e, VId id) const {
  assert(id < readers_.size());
  return find(readers_[id].begin(), readers_[id].end(), e);
}

DataPlane::reader_iterator DataPlane::reader_begin(VId id) const {
  assert(id < readers_.size());
  return readers_[id].begin();
}

DataPlane::reader_iterator DataPlane::reader_end(VId id) const {
  assert(id < readers_.size());
  return readers_[id].end();
}

void DataPlane::register_writer(Engine* e, VId id) {
  assert(id < writers_.size());
  if (writer_find(e, id) == writer_end(id)) {
    writers_[id].push_back(e);
  }
}

void DataPlane::unregister_writer(Engine* e, VId id) {
  assert(id < writers_.size());
  if (writer_find(e, id) != writer_end(id)) {
    writers_[id].erase(writer_find(e, id));
  }
}

DataPlane::writer_iterator DataPlane::writer_find(Engine* e, VId id) const {
  assert(id < writers_.size());
  return find(writers_[id].begin(), writers_[id].end(), e);
}

DataPlane::writer_iterator DataPlane::writer_begin(VId id) const {
  assert(id < writers_.size());
  return writers_[id].begin();
}

DataPlane::writer_iterator DataPlane::writer_end(VId id) const {
  assert(id < writers_.size());
  return writers_[id].end();
}

void DataPlane::write(VId id, const Bits* bits) {
  assert(id < readers_.size());
  assert(id < write_buf_.size());

  if (write_buf_[id] == *bits) {
    return;
  } 
  write_buf_[id] = *bits;
  for (auto e : readers_[id]) {
    e->read(id, &write_buf_[id]);
  } 
}

void DataPlane::write(VId id, bool b) {
  assert(id < readers_.size());
  assert(id < write_buf_.size());

  if (write_buf_[id].to_bool() == b) {
    return;
  } 
  write_buf_[id].flip(0);
  for (auto e : readers_[id]) {
    e->read(id, &write_buf_[id]);
  } 
}

} // namespace cascade
