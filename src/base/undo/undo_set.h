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

#ifndef CASCADE_SRC_BASE_UNDO_UNDO_SET_H
#define CASCADE_SRC_BASE_UNDO_UNDO_SET_H

#include <cassert>
#include <unordered_set>
#include "src/base/undo/undoable.h"

namespace cascade {

// The UndoSet class represents a simple undoable store for immutable keys. The
// ManagedUndoSet class is designed to be used with pointers that the user does
// not want to spend time deallocating. Objects are freed on teardown and when
// the undo() method is invoked.

template <typename K, typename H, typename E>
class BaseUndoSet : public Undoable {
  public:
    // Iterators:
    typedef typename std::unordered_set<K,H,E>::const_iterator const_iterator;

    // Constructors:
    BaseUndoSet(size_t bs, const H& h, const E& e);
    ~BaseUndoSet() override = default;

    // Undoable Interface:
    void checkpoint() override;
    void commit() override;

    // User Interface:
    bool empty() const;
    size_t size() const;
    const_iterator insert(K k);
    const_iterator find(K k) const;
    const_iterator begin() const;
    const_iterator end() const;

  protected:
    std::unordered_set<K,H,E> set_;
    std::unordered_set<K,H,E> deltas_;
};

template <typename K, typename H = std::hash<K>, typename E = std::equal_to<K>>
class UndoSet : public BaseUndoSet<K,H,E> {
  public:
    // Constructors:
    UndoSet(size_t bs = 16, const H& h = H(), const E& e = E());
    ~UndoSet() override = default;

    // Undoable Interface:
    void undo() override;
};

template <typename K, typename H = std::hash<K>, typename E = std::equal_to<K>>
class ManagedUndoSet : public BaseUndoSet<K,H,E> {
  public:
    // Constructors:
    ManagedUndoSet(size_t bs = 16, const H& h = H(), const E& e = E());
    ~ManagedUndoSet() override;

    // Undoable Interface:
    void undo() override;
};

template <typename K, typename H, typename E>
inline BaseUndoSet<K,H,E>::BaseUndoSet(size_t bs, const H& h, const E& e) : set_(bs,h,e), deltas_(bs,h,e) {
  // Does nothing.
}

template <typename K, typename H, typename E>
inline void BaseUndoSet<K,H,E>::checkpoint() {
  // Does nothing.
}

template <typename K, typename H, typename E>
inline void BaseUndoSet<K,H,E>::commit() {
  deltas_.clear();
}

template <typename K, typename H, typename E>
inline bool BaseUndoSet<K,H,E>::empty() const {
  return set_.empty();
}

template <typename K, typename H, typename E>
inline size_t BaseUndoSet<K,H,E>::size() const {
  return set_.size();
}

template <typename K, typename H, typename E>
inline typename BaseUndoSet<K,H,E>::const_iterator BaseUndoSet<K,H,E>::insert(K k) {
  assert(set_.find(k) == set_.end());
  assert(deltas_.find(k) == deltas_.end());
  deltas_.insert(k);
  return set_.insert(k).first;
}

template <typename K, typename H, typename E>
inline typename BaseUndoSet<K,H,E>::const_iterator BaseUndoSet<K,H,E>::find(K k) const {
  return set_.find(k);
}

template <typename K, typename H, typename E>
inline typename BaseUndoSet<K,H,E>::const_iterator BaseUndoSet<K,H,E>::begin() const {
  return set_.begin();
}

template <typename K, typename H, typename E>
inline typename BaseUndoSet<K,H,E>::const_iterator BaseUndoSet<K,H,E>::end() const {
  return set_.end();
}

template <typename K, typename H, typename E>
inline UndoSet<K,H,E>::UndoSet(size_t bs, const H& h, const E& e) : BaseUndoSet<K,H,E>(bs,h,e) {
  // Does nothing.
}

template <typename K, typename H, typename E>
inline void UndoSet<K,H,E>::undo() {
  for (auto d : this->deltas_) {
    assert(this->set_.find(d) != this->set_.end());
    this->set_.erase(d);
  }
  this->deltas_.clear();
}

template <typename K, typename H, typename E>
inline ManagedUndoSet<K,H,E>::ManagedUndoSet(size_t bs, const H& h, const E& e) : BaseUndoSet<K,H,E>(bs,h,e) {
  // Does nothing.
}

template <typename K, typename H, typename E>
inline ManagedUndoSet<K,H,E>::~ManagedUndoSet() {
  for (auto k : this->keys_) {
    delete k;
  }
}

template <typename K, typename H, typename E>
inline void ManagedUndoSet<K,H,E>::undo() {
  for (auto d : this->deltas_) {
    const auto itr = this->set_.find(d);
    assert(itr != this->set_.end());
    delete *itr;
    this->set_.erase(itr);
  }
  this->deltas_.clear();
}

} // namespace cascade

#endif
