#ifndef CASCADE_SRC_CL_SINGLETON_H
#define CASCADE_SRC_CL_SINGLETON_H

namespace cascade::cl {

template <typename T>
struct Singleton {
  typedef T value_type;

  Singleton() = delete;

  template <typename ...Args>
  static T& get(Args... args) {
    static T t{args...};
    return t;
  }
};

} // namespace cascade::cl

#endif
