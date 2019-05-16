#ifndef CASCADE_SRC_CL_FILE_ARG_H
#define CASCADE_SRC_CL_FILE_ARG_H

#include <fstream>
#include "comment_stream.h"
#include "str_arg.h"

namespace cascade::cl {

template <typename T, typename R>
struct FileReader {
  bool operator()(std::istream& is, T& t) const {
    std::string path = "";
    is >> path;
    std::ifstream ifs(path);
    comment_stream cs(ifs);
    return ifs.is_open() ? R()(cs, t) : false;
  }
};

template <typename T, typename R = StrReader<T>, typename W = StrWriter<T>>
using FileArg = StrArg<T, FileReader<T, R>, W>; 

} // namespace cascade::cl

#endif
