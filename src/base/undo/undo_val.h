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

#ifndef CASCADE_SRC_BASE_UNDO_UNDO_VAL_H
#define CASCADE_SRC_BASE_UNDO_UNDO_VAL_H

#include "src/base/undo/undoable.h"

namespace cascade {

template <typename T>
class BaseUndoVal : public Undoable {
  public:
    BaseUndoVal();
    ~BaseUndoVal() override = default;

    const T& get() const;

    void checkpoint() override;
    void commit() override;

  protected:
    T t_;
    T backup_;
};

template <typename T>
class UndoVal : public BaseUndoVal<T> {
  public:
    UndoVal();
    UndoVal(const T& rhs);
    UndoVal& operator=(const T& rhs);
    ~UndoVal() override = default;

    void undo() override;
};

template <typename T>
class ManagedUndoVal : public BaseUndoVal<T> {
  public:
    ManagedUndoVal();
    ManagedUndoVal(const T& rhs);
    ManagedUndoVal& operator=(const T& rhs);
    ~ManagedUndoVal() override;

    void undo() override;
};

template <typename T>
inline BaseUndoVal<T>::BaseUndoVal() : Undoable() { }

template <typename T>
inline const T& BaseUndoVal<T>::get() const {
  return t_;
}

template <typename T>
inline void BaseUndoVal<T>::checkpoint() {
  backup_ = t_;
}

template <typename T>
inline void BaseUndoVal<T>::commit() {
  // Does nothing.
}

template <typename T>
inline UndoVal<T>::UndoVal() : BaseUndoVal<T>() { }

template <typename T>
inline UndoVal<T>::UndoVal(const T& rhs) : BaseUndoVal<T>() {
  this->t_ = rhs;
}

template <typename T>
inline UndoVal<T>& UndoVal<T>::operator=(const T& rhs) {
  this->t_ = rhs;
  return *this;
}

template <typename T>
inline void UndoVal<T>::undo() {
  this->t_ = this->backup_;
}
template <typename T>
inline ManagedUndoVal<T>::ManagedUndoVal() : BaseUndoVal<T>() { }

template <typename T>
inline ManagedUndoVal<T>::ManagedUndoVal(const T& rhs) : BaseUndoVal<T>() {
  this->t_ = rhs;
}

template <typename T>
inline ManagedUndoVal<T>& ManagedUndoVal<T>::operator=(const T& rhs) {
  this->t_ = rhs;
  return *this;
}

template <typename T>
inline ManagedUndoVal<T>::~ManagedUndoVal() {
  if (this->t_ != nullptr) {
    delete this->t_;
  }
}

template <typename T>
inline void ManagedUndoVal<T>::undo() {
  if (this->t_ != this->backup_ && this->t_ != nullptr) {
    delete this->t_;
  }
  this->t_ = this->backup_;
}

} // namespace cascade

#endif
