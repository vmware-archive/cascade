#ifndef NFA_H
#define NFA_H

#include <iostream>
#include <map>
#include <set>

namespace ns {

class Nfa {
  public:
    // Creates an nfa with a single entry
    Nfa();
    // Creates an nfa that accepts a character
    Nfa(char c);
    // Destructor
    ~Nfa();

    // Thompson's Construction Interface:
    void concat(Nfa* rhs);
    void disjoin(Nfa* rhs);
    void plus();
    void qmark();
    void star();

    // Minimization Interface:
    void make_deterministic();

    // Printing Interface:
    void to_text(std::ostream& os) const;
    void to_verilog(std::ostream& os) const;

  private:
    struct State {
      void connect(char c, State* s);
      std::map<char, std::set<State*>> ts;
    };
    typedef typename std::set<State*> PowerState;
    typedef typename std::set<char> TransitionSet;
      
    // Thompson's Construction Helpers:
    char epsilon() const;
    State* get_state();
    bool is_entry(State* s) const;
    bool is_accept(State* s) const;

    // Nfa to Dfa Helpers:
    PowerState epsilon_closure(const PowerState& ps) const;
    PowerState power_entry() const;
    bool is_power_accept(const PowerState& ps) const;
    TransitionSet power_transitions(const PowerState& ps) const;
    PowerState power_transition(const PowerState& ps, char c) const;
    Nfa* powerset_construction() const;

    // State:
    std::set<State*> states_; 
    State* entry_;
    std::set<State*> accepts_;
};

} // namespace ns

#endif
