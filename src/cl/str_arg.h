#ifndef CASCADE_SRC_CL_STR_ARG_H
#define CASCADE_SRC_CL_STR_ARG_H

#include "arg.h"

namespace cascade::cl {

template <typename T>
struct StrReader {
  bool operator()(std::istream& is, T& t) const {
    is >> t;
    return !is.fail();
  }
};  
template <typename T>
struct StrWriter {
  void operator()(std::ostream& os, const T& t) const {
    os << t;
  }
};

template <typename T, typename R = StrReader<T>, typename W = StrWriter<T>, size_t Arity = 1>
class StrArg : public Arg {
  public:
    static StrArg& create(const std::string& name) {
      return *(new StrArg(name));
    }

    StrArg& alias(const std::string& a) {
      names_.insert(a);
      return *this;
    }
    StrArg& description(const std::string& d) {
      desc_ = d;
      return *this;
    }
    StrArg& usage(const std::string& u) {
      usage_ = u;
      return *this;
    }
    StrArg& required() {
      req_ = true;
      return *this;
    }
    StrArg& initial(const T& val) {
      val_ = val;
      return *this;
    }
    const T& value() const {
      return val_;
    }
    operator const T&() const {
      return val_;
    }

    void read(std::istream& is) override {
      err_ = !R()(is, val_);
      dup_ = prov_;
      prov_ = true;
    }
    void write(std::ostream& os) const override {
      W()(os, val_);
    }
    size_t arity() const override {
      return Arity;
    }

  private:
    StrArg(const std::string& name) : Arg(name), val_() {}
    T val_;
};

} // namespace cascade::cl

#endif
