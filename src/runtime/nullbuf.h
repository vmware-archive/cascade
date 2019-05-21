#ifndef CASCADE_SRC_RUNTIME_NULLBUF_H
#define CASCADE_SRC_RUNTIME_NULLBUF_H

#include <streambuf>

namespace cascade {

class nullbuf : public std::streambuf {
  public:
    // Typedefs:
    typedef std::streambuf::char_type char_type;
    typedef std::streambuf::traits_type traits_type;
    typedef std::streambuf::int_type int_type;
    typedef std::streambuf::pos_type pos_type;
    typedef std::streambuf::off_type off_type;
   
    // Constructors:
    ~nullbuf() override = default;

  private:
    std::streamsize xsputn(const char_type* s, std::streamsize count) override;
    int_type overflow(int_type c = traits_type::eof()) override;

    int_type underflow() override;
    int_type uflow() override;
    std::streamsize xsgetn(char_type* s, std::streamsize count) override;
};

inline std::streamsize nullbuf::xsputn(const char_type* s, std::streamsize count) {
  (void) s;
  return count;
}

inline nullbuf::int_type nullbuf::overflow(int_type c) {
  return c;
}

inline nullbuf::int_type nullbuf::underflow() {
  return traits_type::eof();
}

inline nullbuf::int_type nullbuf::uflow() {
  return traits_type::eof();
}

inline std::streamsize nullbuf::xsgetn(char_type* s, std::streamsize count) {
  (void) s;
  (void) count;
  return 0;
}

} // namespace 

#endif
