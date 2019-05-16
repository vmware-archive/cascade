#ifndef CASCADE_SRC_CL_GROUP_H
#define CASCADE_SRC_CL_GROUP_H

#include <algorithm>
#include "arg_table.h"
#include "singleton.h"

namespace cascade::cl {

class Group {
  public:
    static Group& create(const std::string& name) {
      return *(new Group(name));
    }

    Group(const Group& rhs) = delete;
    Group(const Group&& rhs) = delete;
    Group& operator=(Group& rhs) = delete;
    Group& operator=(Group&& rhs) = delete;

    const std::string& name() const {
      return name_;
    }
    typedef std::vector<Arg*>::const_iterator arg_itr;
    arg_itr arg_begin() const {
      return Singleton<ArgTable>::get().args_by_group_[idx_].begin();
    }
    arg_itr arg_end() const {
      return Singleton<ArgTable>::get().args_by_group_[idx_].end();
    }

  private:
    Group(const std::string& name) : name_(name) { 
      auto& table = Singleton<ArgTable>::get();
      table.groups_.push_back(this);
      idx_ = table.args_by_group_.size();
      table.args_by_group_.resize(idx_+1);
    }

    std::string name_;
    size_t idx_;
};

} // namespace cascade::cl

#endif
