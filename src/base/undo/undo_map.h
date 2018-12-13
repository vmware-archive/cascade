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

#ifndef CASCADE_SRC_MISC_UNDO_MAP_H
#define CASCADE_SRC_MISC_UNDO_MAP_H

#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include "src/base/undo/undoable.h"

namespace cascade {

// The UndoMap class represents a simple undoable store for immutable key value
// pairs. The ManagedUndoMap class is designed to be used with value pointers
// that the user does not want to spend time deallocating. Objects are freed on
// teardown and when the undo() method is invoked.

template <typename K, typename V, typename H, typename E>
class BaseUndoMap : public Undoable {
  public:
    // Iterators:
    typedef typename std::unordered_map<K,V,H,E>::const_iterator const_iterator;

    BaseUndoMap(size_t bs, const H& h, const E& e);
    ~BaseUndoMap() override = default;

    // Undoable Interface:
    void checkpoint() override;
    void commit() override;

    // User Interface:
    bool empty() const;
    size_t size() const;
    const_iterator insert(K k, V v);
    const_iterator find(K k) const;
    const_iterator begin() const;
    const_iterator end() const;

  protected:
    std::unordered_map<K,V,H,E> map_;
    std::unordered_set<K,H,E> deltas_;
};

template <typename K, typename V, typename H = std::hash<K>, typename E = std::equal_to<K>>
class UndoMap : public BaseUndoMap<K,V,H,E> {
  public:
    // Constructors:
    UndoMap(size_t bs = 16, const H& h = H(), const E& e = E());
    ~UndoMap() override = default;

    // Undoable Interface:
    void undo() override;
};

template <typename K, bool MK, typename V, bool MV, typename H = std::hash<K>, typename E = std::equal_to<K>>
class ManagedUndoMap : public BaseUndoMap<K,V,H,E> {
  public:
    // Constructors:
    ManagedUndoMap(size_t bs = 16, const H& h = H(), const E& e = E());
    ~ManagedUndoMap() override;

    // Undoable Interface:
    void undo() override;
};

template <typename K, typename V, typename H, typename E>
inline BaseUndoMap<K,V,H,E>::BaseUndoMap(size_t bs, const H& h, const E& e) : map_(bs,h,e), deltas_(bs,h,e) {
  // Does nothing.
}

template <typename K, typename V, typename H, typename E>
inline void BaseUndoMap<K,V,H,E>::checkpoint() {
  // Does nothing.
}

template <typename K, typename V, typename H, typename E>
inline void BaseUndoMap<K,V,H,E>::commit() {
  deltas_.clear();
}

template <typename K, typename V, typename H, typename E>
inline bool BaseUndoMap<K,V,H,E>::empty() const {
  return map_.empty();
}

template <typename K, typename V, typename H, typename E>
inline size_t BaseUndoMap<K,V,H,E>::size() const {
  return map_.size();
}

template <typename K, typename V, typename H, typename E>
inline typename BaseUndoMap<K,V,H,E>::const_iterator BaseUndoMap<K,V,H,E>::insert(K k, V v) {
  assert(map_.find(k) == map_.end());
  assert(deltas_.find(k) == deltas_.end());
  deltas_.insert(k);
  return map_.insert(std::make_pair(k,v)).first;
}

template <typename K, typename V, typename H, typename E>
inline typename BaseUndoMap<K,V,H,E>::const_iterator BaseUndoMap<K,V,H,E>::find(K k) const {
  return map_.find(k);
}

template <typename K, typename V, typename H, typename E>
inline typename BaseUndoMap<K,V,H,E>::const_iterator BaseUndoMap<K,V,H,E>::begin() const {
  return map_.begin();
}

template <typename K, typename V, typename H, typename E>
inline typename BaseUndoMap<K,V,H,E>::const_iterator BaseUndoMap<K,V,H,E>::end() const {
  return map_.end();
}

template <typename K, typename V, typename H, typename E>
inline UndoMap<K,V,H,E>::UndoMap(size_t bs, const H& h, const E& e) : BaseUndoMap<K,V,H,E>(bs,h,e) {
  // Does nothing.
}

template <typename K, typename V, typename H, typename E>
inline void UndoMap<K,V,H,E>::undo() {
  for (auto d : this->deltas_) {
    assert(this->map_.find(d) != this->map_.end());
    this->map_.erase(d);
  }
  this->deltas_.clear();
}

template <typename K, bool MK, typename V, bool MV, typename H, typename E>
inline ManagedUndoMap<K,MK,V,MV,H,E>::ManagedUndoMap(size_t bs, const H& h, const E& e) : BaseUndoMap<K,V,H,E>(bs,h,e) {
  // Does nothing.
}

template <typename K, bool MK, typename V, bool MV, typename H, typename E>
inline ManagedUndoMap<K,MK,V,MV,H,E>::~ManagedUndoMap() {
  for (auto m : this->map_) {
    if (MK) {
      delete m.first;
    }
    if (MV) {
      delete m.second;
    }
  }
}

template <typename K, bool MK, typename V, bool MV, typename H, typename E>
inline void ManagedUndoMap<K,MK,V,MV,H,E>::undo() {
  for (auto d : this->deltas_) {
    const auto itr = this->map_.find(d);
    assert(itr != this->map_.end());
    if (MK) {
      delete itr->first;
    }
    if (MV) {
      delete itr->second;
    }
    this->map_.erase(itr);
  }
  this->deltas_.clear();
}

} // namespace cascade

#endif
