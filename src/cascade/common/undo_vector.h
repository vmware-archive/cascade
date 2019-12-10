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

#ifndef CASCADE_SRC_COMMON_UNDO_VECTOR_H
#define CASCADE_SRC_COMMON_UNDO_VECTOR_H

#include <vector>
#include "base/undo/undoable.h"

namespace cascade {

// The UndoVector class represents a simple extensible array for immutable
// objects.  The ManagedUndoVector class is designed to be used with pointers
// that the user does not want to spend time deallocating. Objects are freed on
// teardown and when the undo() method is invoked.

template <typename T>
class BaseUndoVector : public Undoable {
  public:
    // Iterators:
    typedef typename std::vector<T>::const_iterator const_iterator;

    // Constructors:
    BaseUndoVector();
    ~BaseUndoVector() override = default;

    // Undoable Interface:
    void checkpoint() override;
    void commit() override;

    // User Interface:
    bool empty() const;
    size_t size() const;
    void push_back(const T& t);
    const T& operator[](size_t n) const;
    const_iterator begin() const;
    const_iterator end() const;

  protected:
    std::vector<T> vec_;
    size_t n_;
};

template <typename T>
class UndoVector : public BaseUndoVector<T> {
  public:
    // Constructors:
    UndoVector();
    ~UndoVector() override = default;

    // Undoable Interface:
    void undo() override;
};

template <typename T>
class ManagedUndoVector : public BaseUndoVector<T> {
  public:
    // Constructors:
    ManagedUndoVector();
    ~ManagedUndoVector() override;

    // Undoable Interface:
    void undo() override;
};

template <typename T>
inline BaseUndoVector<T>::BaseUndoVector() : Undoable() { 
  n_ = 0;
}

template <typename T>
inline void BaseUndoVector<T>::checkpoint() {
  // Does nothing.
}

template <typename T>
inline void BaseUndoVector<T>::commit() {
  n_ = vec_.size();
}

template <typename T>
inline bool BaseUndoVector<T>::empty() const {
  return vec_.empty();
}

template <typename T>
inline size_t BaseUndoVector<T>::size() const {
  return vec_.size();
}

template <typename T>
inline void BaseUndoVector<T>::push_back(const T& t) {
  vec_.push_back(t);
}

template <typename T>
inline const T& BaseUndoVector<T>::operator[](size_t n) const {
  assert(n < size());
  return vec_[n];
}

template <typename T>
inline typename BaseUndoVector<T>::const_iterator BaseUndoVector<T>::begin() const {
  return vec_.begin();
}

template <typename T>
inline typename BaseUndoVector<T>::const_iterator BaseUndoVector<T>::end() const {
  return vec_.end();
}

template <typename T>
inline UndoVector<T>::UndoVector() : BaseUndoVector<T>() { }

template <typename T>
inline void UndoVector<T>::undo() {
  this->vec_.resize(this->n_);
}

template <typename T>
inline ManagedUndoVector<T>::ManagedUndoVector() : BaseUndoVector<T>() { }

template <typename T>
inline ManagedUndoVector<T>::~ManagedUndoVector() {
  for (auto t : this->vec_) {
    delete t;
  }
}

template <typename T>
inline void ManagedUndoVector<T>::undo() {
  for (size_t i = this->n_, ie = this->vec_.size(); i < ie; ++i) {
    delete this->vec_[i];
  }
  this->vec_.resize(this->n_);
}

} // namespace cascade

#endif
