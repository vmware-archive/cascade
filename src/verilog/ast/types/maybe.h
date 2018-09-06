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

#ifndef CASCADE_SRC_VERILOG_AST_MAYBE_H
#define CASCADE_SRC_VERILOG_AST_MAYBE_H

#include <functional>
#include "src/verilog/ast/types/combinator.h"

namespace cascade {

template <typename T>
class Maybe : public Combinator {
  public:
    // Constructors:
    Maybe(T* t = nullptr);
    ~Maybe() override;

    // Node Inteface:
    Maybe* clone() const override;
    void accept(Visitor* v) const override;
    void accept(Editor* e) override;
    Maybe* accept(Builder* b) const override;
    Maybe* accept(Rewriter* r) override;

    // Query Interface:
    bool null() const;

    // Get/Set:
    T* get();
    const T* get() const;
    void set(T* t);
    void replace(T* t);
    void conditional_replace(T* t);

    // Convenience Operators:
    void accept(Visitor* v, std::function<void()> pre, std::function<void()> post) const;

  private:
    T* t_;
};

template <typename T>
inline Maybe<T>::Maybe(T* t) {
  parent_ = nullptr;
  t_ = t;
  if (t_ != nullptr) {
    t_->parent_ = this;
  }
}

template <typename T>
inline Maybe<T>::~Maybe() {
  if (t_ != nullptr) {
    delete t_;
  }
}

template <typename T>
inline Maybe<T>* Maybe<T>::clone() const {
  return new Maybe(t_ == nullptr ? nullptr : t_->clone());
}

template <typename T>
inline void Maybe<T>::accept(Visitor* v) const {
  if (t_ != nullptr) {
    t_->accept(v);
  }
}

template <typename T>
inline void Maybe<T>::accept(Editor* e) {
  if (t_ != nullptr) {
    t_->accept(e);
  }
}

template <typename T>
inline Maybe<T>* Maybe<T>::accept(Builder* b) const {
  return new Maybe(t_ == nullptr ? nullptr : t_->accept(b));
}

template <typename T>
inline Maybe<T>* Maybe<T>::accept(Rewriter* r) {
  if (t_ != nullptr) {
    conditional_replace(t_->accept(r));
  }
  return this;
}

template <typename T>
inline bool Maybe<T>::null() const {
  return t_ == nullptr;
}

template <typename T>
inline T* Maybe<T>::get() {
  return t_;
}

template <typename T>
inline const T* Maybe<T>::get() const {
  return t_;
}

template <typename T>
inline void Maybe<T>::set(T* t) {
  if (t_ != nullptr) {
    t_->parent_ = nullptr;
  }
  t_ = t;
  if (t_ != nullptr) {
    t_->parent_ = this;
  }
}

template <typename T>
inline void Maybe<T>::replace(T* t) {
  if (t_ != nullptr) {
    delete t_;
  }
  t_ = t;
  if (t_ != nullptr) {
    t_->parent_ = this;
  }
}

template <typename T>
inline void Maybe<T>::conditional_replace(T* t) {
  if (t_ != t) {
    replace(t);
  }
}

template <typename T>
inline void Maybe<T>::accept(Visitor* v, std::function<void()> pre, std::function<void()> post) const {
  if (t_ != nullptr) {
    pre();
    t_->accept(v);
    post();
  }
}

} // namespace cascade

#endif
