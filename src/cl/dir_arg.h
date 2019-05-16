#ifndef CASCADE_SRC_CL_DIR_ARG_H
#define CASCADE_SRC_CL_DIR_ARG_H

#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include "comment_stream.h"
#include "str_arg.h"

namespace cascade::cl {

template <typename T, typename R>
class DirReader {
  public:
    bool operator()(std::istream& is, T& t) const {
      std::stringstream ss;
      std::string path = "";
      is >> path;
      return walk(ss, path) ? R()(ss, t) : false;
    }
  
  private:
    bool walk(std::stringstream& ss, const std::string& dir) const {
      DIR* dp = opendir(dir.c_str());    
      if (dp == NULL) {
        return false;
      }
      while (dirent* de = readdir(dp)) {
        const std::string file = de->d_name;
        if (file == "." || file == "..") {
          continue;
        }
        const auto path = dir + "/" + file;
        struct stat filestat;
        if (stat(path.c_str(), &filestat)) {
          return false;
        } 

        if (S_ISDIR(filestat.st_mode)) {
          if (!walk(ss, path)) {
            return false;
          }
        } else {
          std::ifstream ifs(path);
          if (!ifs.is_open()) {
            return false;
          }
          comment_stream cs(ifs);
          ss << cs.rdbuf();
        }
      }
      return true;
    }
};

template <typename T, typename R = StrReader<T>, typename W = StrWriter<T>>
using DirArg = StrArg<T, DirReader<T, R>, W>;

} // namespace cascade::cl

#endif
