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

#ifndef CASCADE_SRC_BASE_CONTAINER_VECTOR_H
#define CASCADE_SRC_BASE_CONTAINER_VECTOR_H

#include <algorithm>
#include <cassert>
#include <stddef.h>
#include <stdint.h>

namespace cascade {

template <typename T>
class Vector {
  public:
    typedef size_t size_type;
    typedef ptrdiff_t	difference_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T* iterator;
    typedef const T* const_iterator; 
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T	value_type;

    Vector();
    Vector(size_type n, const value_type& v = value_type());
    Vector(const Vector& rhs);
    Vector(Vector&& rhs);
    Vector& operator=(Vector rhs);
    ~Vector();

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    size_type size() const;
    void resize(size_type n, const value_type& v = value_type());
    size_type capacity() const;
    bool empty() const;
    void reserve(size_type n);

    reference operator[](size_t idx);
    const_reference operator[](size_t idx) const;

    reference front();
    const_reference front() const;
    reference back();
    const_reference back() const;

    void push_back(const value_type& v);
    void pop_back();

    iterator insert(iterator itr, const value_type& v);
    iterator insert(iterator itr, size_type n, const value_type& v);
    template <typename Itr>
    iterator insert(iterator itr, Itr rb, Itr re);

    iterator erase(iterator itr);

    void swap(Vector& rhs);
    void clear();

  private:
    T* ts_;
    uint32_t size_;
    uint32_t capacity_;    
};

template <typename T>
inline Vector<T>::Vector() {
  ts_ = nullptr; 
  size_ = 0;
  capacity_ = 0;
}

template <typename T>
inline Vector<T>::Vector(size_t n, const value_type& val) : Vector() {
  insert(end(), n, val);
}

template <typename T>
inline Vector<T>::Vector(const Vector& rhs) : Vector() {
  insert(end(), rhs.begin(), rhs.end());
}

template <typename T>
inline Vector<T>::Vector(Vector&& rhs) : Vector() {
  swap(rhs);
}

template <typename T>
inline Vector<T>& Vector<T>::operator=(Vector rhs) {
  swap(rhs);
  return *this;
}

template <typename T>
inline Vector<T>::~Vector() {
  if (ts_ != nullptr) {
    delete[] ts_;
  }
}

template <typename T>
inline typename Vector<T>::iterator Vector<T>::begin() {
  return ts_;
}

template <typename T>
inline typename Vector<T>::const_iterator Vector<T>::begin() const {
  return ts_;
}

template <typename T>
inline typename Vector<T>::iterator Vector<T>::end() {
  return ts_ + size_;
}

template <typename T>
inline typename Vector<T>::const_iterator Vector<T>::end() const {
  return ts_ + size_;
}

template <typename T>
inline typename Vector<T>::size_type Vector<T>::size() const {
  return size_;
}

template <typename T>
inline void Vector<T>::resize(size_type n, const value_type& v) {
  if (n <= size_) {
    size_ = n;
  } else {
    insert(end(), n - size_, v);
  }
}

template <typename T>
inline typename Vector<T>::size_type Vector<T>::capacity() const {
  return capacity_;
}

template <typename T>
inline bool Vector<T>::empty() const {
  return size_ == 0;
}

template <typename T>
inline void Vector<T>::reserve(size_type n) {
  if (capacity_ >= n) {
    return;
  }
  auto new_ts = new T[n];
  if (ts_ != nullptr) {
    std::copy(ts_, ts_ + size_, new_ts);
    delete[] ts_;
  }
  ts_ = new_ts; 
  capacity_ = n;
}

template <typename T>
inline typename Vector<T>::reference Vector<T>::operator[](size_t idx) {
  assert(idx < size_);
  return ts_[idx];
}

template <typename T>
inline typename Vector<T>::const_reference Vector<T>::operator[](size_t idx) const {
  assert(idx < size_);
  return ts_[idx];
}

template <typename T>
inline typename Vector<T>::reference Vector<T>::front() {
  assert(size_ > 0);
  return ts_[0];
}

template <typename T>
inline typename Vector<T>::const_reference Vector<T>::front() const {
  assert(size_ > 0);
  return ts_[0];
}

template <typename T>
inline typename Vector<T>::reference Vector<T>::back() {
  assert(size_ > 0);
  return ts_[size_ - 1];
}

template <typename T>
inline typename Vector<T>::const_reference Vector<T>::back() const {
  assert(size_ > 0);
  return ts_[size_ - 1];
}

template <typename T>
inline void Vector<T>::push_back(const value_type& v) {
  if (size_ < capacity_) {
    ts_[size_++] = v;
  } else {
    insert(end(), v);
  }
}

template <typename T>
inline void Vector<T>::pop_back() {
  assert(size_ > 0);
  --size_;
}

template <typename T>
inline typename Vector<T>::iterator Vector<T>::insert(iterator itr, const value_type& v) {
  assert(itr >= begin());
  assert(itr <= end());

  if (itr == nullptr) {
    reserve(1);
    ts_[0] = v;
    size_ = 1;
    return end();
  }

  const auto delta = itr - ts_;
  reserve(size_ + 1);
  itr = begin() + delta;   

  std::copy(itr, end(), itr + 1);
  *itr++ = v;
  ++size_;

  return itr;
}

template <typename T>
inline typename Vector<T>::iterator Vector<T>::insert(iterator itr, size_type n, const value_type& v) {
  assert(itr >= begin());
  assert(itr <= end());

  if (itr == nullptr) {
    reserve(n);
    std::fill_n(begin(), n, v);
    size_ = n;
    return end();
  }

  const auto delta = itr - ts_;
  reserve(size_ + n);
  itr = begin() + delta;

  std::copy(itr, end(), itr+n);
  std::fill_n(itr, n, v);
  size_ += n;

  return itr + n;
}

template <typename T>
template <typename Itr>
inline typename Vector<T>::iterator Vector<T>::insert(iterator itr, Itr rb, Itr re) {
  assert(itr >= begin());
  assert(itr <= end());
  assert(re >= rb);

  const size_type n = re-rb;
  if (n == 0) {
    return itr;
  }

  if (itr == nullptr) {
    reserve(n);
    std::copy(rb, re, begin());
    size_ = n;
    return end();
  }

  const auto delta = itr - ts_;
  reserve(size_ + n);
  itr = begin() + delta;

  std::copy(itr, end(), itr+n);
  std::copy(rb, re, itr);
  size_ += n;

  return itr + n;
}

template <typename T>
inline typename Vector<T>::iterator Vector<T>::erase(iterator itr) {
  assert(itr >= begin());
  assert(itr < end());

  std::copy(itr + 1, end(), itr);
  --size_; 
  return itr;
}

template <typename T>
inline void Vector<T>::swap(Vector& rhs) {
  std::swap(ts_, rhs.ts_);
  std::swap(size_, rhs.size_);
  std::swap(capacity_, rhs.capacity_);
}

template <typename T>
inline void Vector<T>::clear() {
  size_ = 0;
}

} // namespace cascade

#endif

