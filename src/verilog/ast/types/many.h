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

#ifndef CASCADE_SRC_VERILOG_AST_MANY_H
#define CASCADE_SRC_VERILOG_AST_MANY_H

#include <cassert>
#include <functional>
#include <vector>
#include "src/verilog/ast/types/combinator.h"

namespace cascade {

template <typename T>
class Many : public Combinator {
  public:
    // Typedefs:
    typedef typename std::vector<T*>::const_iterator const_iterator;

    // Constructors:
    Many();
    Many(T* t);
    ~Many() override;

    // Node Interface:
    Many* clone() const override;
    void accept(Visitor* v) const override;
    void accept(Editor* e) override;
    Many* accept(Builder* b) const override;
    Many* accept(Rewriter* r) override;

    // Size:
    size_t size() const;
    bool empty() const;
    void clear();
    const_iterator purge(const_iterator itr);
    void purge_to(size_t n); 

    // Iterators:
    const_iterator begin() const;
    const_iterator end() const;

    // Get/Set:
    T* get(size_t idx);
    const T* get(size_t idx) const;
    void set(size_t idx, T* t);
    void replace(size_t idx, T* t);
    void conditional_replace(size_t idx, T* t);

    // Push/Pop:
    void push_front(T* t);
    void push_back(T* t);
    void pop_front();
    void pop_back();
    T* remove_front();
    T* remove_back();

    // Front/Back:
    T* front();
    T* back();
    const T* front() const;
    const T* back() const;

    // Convenience Operators:
    void accept(Visitor* v, std::function<void()> pre, std::function<void()> post) const;
    void concat(Many* rhs);

  private:
    std::vector<T*> ts_;
};

template <typename T>
inline Many<T>::Many() { 
  parent_ = nullptr;
}

template <typename T>
inline Many<T>::Many(T* t) {
  parent_ = nullptr;
  assert(t != nullptr);
  push_back(t);
}

template <typename T>
inline Many<T>::~Many() {
  purge_to(0);
} 

template <typename T>
inline Many<T>* Many<T>::clone() const {
  auto res = new Many();
  for (auto t : ts_) {
    res->push_back(t->clone());
  }
  return res;
}

template <typename T>
inline void Many<T>::accept(Visitor* v) const {
  for (auto t : ts_) {
    t->accept(v);
  }
}

template <typename T>
inline void Many<T>::accept(Editor* e) {
  for (auto t : ts_) {
    t->accept(e);
  }
}

template <typename T>
inline Many<T>* Many<T>::accept(Builder* b) const {
  auto res = new Many();
  for (auto t : ts_) {
    if (auto tt = t->accept(b)) {
      res->push_back(tt);
    }
  }
  return res;
}

template <typename T>
inline Many<T>* Many<T>::accept(Rewriter* r) {
  for (size_t i = 0, ie = size(); i < ie; ++i) {
    conditional_replace(i, get(i)->accept(r));
  }
  return this;
}

template <typename T>
inline size_t Many<T>::size() const {
  return ts_.size();
}

template <typename T>
inline bool Many<T>::empty() const {
  return ts_.empty();
}

template <typename T>
inline void Many<T>::clear() {
  for (auto t : ts_) {
    assert(t != nullptr);
    t->parent_ = nullptr;
  }
  ts_.clear();
}

template <typename T>
inline typename Many<T>::const_iterator Many<T>::purge(const_iterator itr) {
  assert(itr != ts_.end());
  assert(*itr != nullptr);
  delete *itr;
  return ts_.erase(itr);
}

template <typename T>
inline void Many<T>::purge_to(size_t n) {
  while (size() > n) {
    auto t = ts_.back();
    ts_.pop_back();
    assert(t != nullptr);
    delete t;
  }
}

template <typename T>
inline typename Many<T>::const_iterator Many<T>::begin() const {
  return ts_.begin();
}

template <typename T>
inline typename Many<T>::const_iterator Many<T>::end() const {
  return ts_.end();
}

template <typename T>
inline T* Many<T>::get(size_t n) {
  assert(n < size());
  return ts_[n];
}

template <typename T>
inline const T* Many<T>::get(size_t n) const {
  assert(n < size());
  return ts_[n];
}

template <typename T>
inline void Many<T>::set(size_t n, T* t) {
  assert(n < size());
  assert(ts_[n] != nullptr);
  ts_[n]->parent_ = nullptr;
  ts_[n] = t;
  assert(ts_[n] != nullptr);
  ts_[n]->parent_ = this;
}

template <typename T>
inline void Many<T>::replace(size_t n, T* t) {
  assert(ts_[n] != nullptr);
  delete ts_[n];
  ts_[n] = t;
  assert(ts_[n] != nullptr);
  ts_[n]->parent_ = this;
}

template <typename T>
inline void Many<T>::conditional_replace(size_t n, T* t) {
  assert(n < size());
  if (ts_[n] != t) {
    replace(n, t);
  }
}

template <typename T>
inline void Many<T>::push_front(T* t) {
  assert(t != nullptr);
  t->parent_ = this;
  ts_.insert(ts_.begin(), t);
}

template <typename T>
inline void Many<T>::push_back(T* t) {
  assert(t != nullptr);
  t->parent_ = this;
  ts_.push_back(t);
}

template <typename T>
inline void Many<T>::pop_front() {
  assert(!empty());
  assert(ts_.front() != nullptr);
  ts_.front()->parent_ = nullptr;
  ts_.erase(ts_.begin());
}

template <typename T>
inline void Many<T>::pop_back() {
  assert(!empty());
  assert(ts_.back() != nullptr);
  ts_.back()->parent_ = nullptr;
  ts_.pop_back();
}

template <typename T>
inline T* Many<T>::remove_front() {
  auto t = front();
  pop_front();
  return t;
}

template <typename T>
inline T* Many<T>::remove_back() {
  auto t = back();
  pop_back();
  return t;
}

template <typename T>
inline T* Many<T>::front() {
  assert(!empty());
  return ts_.front();
}

template <typename T>
inline T* Many<T>::back() {
  assert(!empty());
  return ts_.back();
}

template <typename T>
inline const T* Many<T>::front() const {
  assert(!empty());
  return ts_.front();
}

template <typename T>
inline const T* Many<T>::back() const {
  assert(!empty());
  return ts_.back();
}

template <typename T>
inline void Many<T>::accept(Visitor* v, std::function<void()> pre, std::function<void()> post) const {
  for (auto t : *this) {
    pre();
    t->accept(v);
    post();
  }
}

template <typename T>
inline void Many<T>::concat(Many* rhs) {
  for (auto t : rhs->ts_) {
    assert(t != nullptr);
    t->parent_ = this;
  }
  ts_.insert(end(), rhs->begin(), rhs->end());
  rhs->ts_.clear();
  delete rhs;
}

} // namespace cascade

#endif
