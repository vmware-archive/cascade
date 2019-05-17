#ifndef CASCADE_SRC_CASCADE_EVALSTREAM_H
#define CASCADE_SRC_CASCADE_EVALSTREAM_H

#include <iostream>
#include <sstream>
#include "runtime/runtime.h"

namespace cascade {

class evalbuf : public std::streambuf {
  public:
    // Typedefs:
    typedef std::streambuf::char_type char_type;
    typedef std::streambuf::traits_type traits_type;
    typedef std::streambuf::int_type int_type;
    typedef std::streambuf::pos_type pos_type;
    typedef std::streambuf::off_type off_type;
   
    // Constructors:
    explicit evalbuf(Runtime* rt);
    ~evalbuf() override = default;

  private:
    Runtime* rt_;
    std::stringstream ss_;

    int_type sync() override;
    std::streamsize xsputn(const char_type* s, std::streamsize count) override;
    int_type overflow(int_type c = traits_type::eof()) override;
};

class evalstream : public std::ostream {
  public:
    evalstream(Runtime* rt);
    ~evalstream() override = default;

  private:
    evalbuf buf_;
};

inline evalbuf::evalbuf(Runtime* rt) : std::streambuf() {
  rt_ = rt;
}

inline evalbuf::int_type evalbuf::sync() {
  rt_->eval(ss_.str());
  ss_.str(std::string());
  return int_type(0);
}

inline std::streamsize evalbuf::xsputn(const char_type* s, std::streamsize count) {
  ss_.write(s, count);
  return count;
}

inline evalbuf::int_type evalbuf::overflow(int_type c) {
  if (c != traits_type::eof()) {
    ss_.put(c);
  }
  return c;
}

inline evalstream::evalstream(Runtime* rt) : std::ostream(&buf_), buf_(rt) { }

} // namespace cascade

#endif
