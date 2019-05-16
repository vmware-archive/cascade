#ifndef CASCADE_SRC_CL_COMMENT_STREAM_H
#define CASCADE_SRC_CL_COMMENT_STREAM_H

#include <iostream>
#include <streambuf>

namespace cascade::cl {

class comment_buf : public std::streambuf {
  public:
    comment_buf(std::streambuf* buf) : std::streambuf(), buf_(buf), on_comment_(false) {}

  protected:
    int_type underflow() override {
      auto res = buf_->sgetc();
      if (res != '#') {
        return on_comment_ ? traits_type::to_int_type(' ') : res;
      }
      do {
        res = buf_->snextc();
      } while (res != traits_type::eof() && res != '\n');
      on_comment_ = res == '\n';
      return on_comment_ ? traits_type::to_int_type(' ') : res; 
    }
    int_type uflow() override {
      const auto res = underflow();
      if (res != traits_type::eof()) {
        buf_->sbumpc();
        on_comment_ = false;
      }
      return res;
    }

  private:
    std::streambuf* buf_;
    bool on_comment_;
};

class comment_stream : public std::istream {
  public:
    comment_stream(std::istream& is) : std::istream(&buf_), buf_(is.rdbuf()) {}
  private:
    comment_buf buf_;
};

} // namespace cascade::cl

#endif
