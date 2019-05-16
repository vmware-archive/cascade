#ifndef CASCADE_SRC_CL_ARG_H
#define CASCADE_SRC_CL_ARG_H

#include <iostream>
#include <set>
#include <string>
#include "arg_table.h"
#include "group.h"
#include "singleton.h"

namespace cascade::cl {

class Arg {
  public:
    Arg(const std::string& name) : names_({{name}}), desc_(""), usage_(""), req_(false), prov_(false), dup_(false), err_(false) {
      auto& table = Singleton<ArgTable>::get();
      if (table.groups_.empty()) {
        Group::create("Ungrouped Arguments");
      }
      table.args_by_group_.back().push_back(this);
      table.args_.push_back(this);
    }
    Arg(const Arg& rhs) = delete;
    Arg(const Arg&& rhs) = delete;
    Arg& operator=(Arg& rhs) = delete;
    Arg& operator=(Arg&& rhs) = delete;
    virtual ~Arg() = default;

    typedef std::set<std::string>::const_iterator alias_itr;
    const alias_itr alias_begin() const {
      return names_.begin();
    }
    const alias_itr alias_end() const {
      return names_.end();
    }
    bool matches(const std::string& alias) const {    
      return names_.find(alias) != names_.end();    
    }
    const std::string& description() const {
      return desc_;
    }
    const std::string& usage() const {
      return usage_;
    }
    bool required() const {
      return req_;
    }
    bool provided() const {
      return prov_;
    }
    bool duplicated() const {
      return dup_;
    }
    bool error() const {
      return err_;
    }

    virtual void read(std::istream& is) = 0;
    virtual void write(std::ostream& os) const = 0;
    virtual size_t arity() const = 0;

  protected:
    std::set<std::string> names_;
    std::string desc_;
    std::string usage_;
    bool req_;
    bool prov_;
    bool dup_;
    bool err_;
};

} // namespace cascade::cl

#endif
