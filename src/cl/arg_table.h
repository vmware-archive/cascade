#ifndef CASCADE_SRC_CL_ARG_TABLE_H
#define CASCADE_SRC_CL_ARG_TABLE_H

#include <vector>

namespace cascade::cl {

class Arg;
class Group;

struct ArgTable {
  std::vector<Arg*> args_;
  std::vector<Group*> groups_;
  std::vector<std::vector<Arg*>> args_by_group_;
  std::vector<const char*> unrec_;
};

} // namespace cascade::cl

#endif
