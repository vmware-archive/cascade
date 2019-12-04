#include "nfa.h"

#include <stack>

using namespace std;

namespace ns {

Nfa::Nfa() {
  auto s1 = get_state();
  entry_ = s1;
}

Nfa::Nfa(char c) {
  auto s1 = get_state();
  auto s2 = get_state();
  s1->connect(c, s2);

  entry_ = s1;
  accepts_.insert(s2);
}

Nfa::~Nfa() {
  for (auto s : states_) {
    delete s;
  }
}

void Nfa::concat(Nfa* rhs) {
  states_.insert(rhs->states_.begin(), rhs->states_.end());
  for (auto a : accepts_) {
    a->connect(epsilon(), rhs->entry_);
  }
  accepts_ = rhs->accepts_;
  
  rhs->states_.clear();
  delete rhs;
}

void Nfa::disjoin(Nfa* rhs) {
  states_.insert(rhs->states_.begin(), rhs->states_.end());

  auto s1 = get_state();
  auto s2 = get_state();
  s1->connect(epsilon(), entry_);
  s1->connect(epsilon(), rhs->entry_);
  for (auto a : accepts_) {
    a->connect(epsilon(), s2);
  }
  for (auto a : rhs->accepts_) {
    a->connect(epsilon(), s2);
  }

  entry_ = s1;
  accepts_.clear();
  accepts_.insert(s2);

  rhs->states_.clear();
  delete rhs;
}

void Nfa::plus() {
  auto s1 = get_state();
  auto s2 = get_state();
  s1->connect(epsilon(), entry_);
  for (auto a : accepts_) {
    a->connect(epsilon(), s2);
    a->connect(epsilon(), entry_);
  }

  entry_ = s1;
  accepts_.clear();
  accepts_.insert(s2);
}

void Nfa::qmark() {
  auto s1 = get_state();
  auto s2 = get_state();
  s1->connect(epsilon(), entry_);
  s1->connect(epsilon(), s2);
  for (auto a : accepts_) {
    a->connect(epsilon(), s2);
  }

  entry_ = s1;
  accepts_.clear();
  accepts_.insert(s2);
}

void Nfa::star() {
  auto s1 = get_state();
  auto s2 = get_state();
  s1->connect(epsilon(), entry_);
  s1->connect(epsilon(), s2);
  for (auto a : accepts_) {
    a->connect(epsilon(), s2);
    a->connect(epsilon(), entry_);
  }

  entry_ = s1;
  accepts_.clear();
  accepts_.insert(s2);
}

void Nfa::make_deterministic() {
  auto dfa = powerset_construction();
  std::swap(states_, dfa->states_);
  std::swap(entry_, dfa->entry_);
  std::swap(accepts_, dfa->accepts_);
  delete dfa;
}

void Nfa::to_text(ostream& os) const {
  map<State*, size_t> ids;
  for (auto s : states_) {
    ids.insert(make_pair(s, ids.size()));
  }  

  for (auto s : states_) {
    os << "state " << ids[s] << ":";
    if (is_entry(s)) {
      os << " (entry)";
    }
    if (is_accept(s)) {
      os << " (accept)";
    }
    os << endl;

    for (auto& t : s->ts) {
      if (t.first == epsilon()) {
        os << "\t<e> -> {";
      } else {
        os << "\t" << t.first << " -> {";
      } 
      for (auto s2 : t.second) {
        os << " " << ids[s2];
      }
      os << " }" << endl;
    }
  } 
}

void Nfa::to_verilog(ostream& os) const {
  map<State*, size_t> ids;
  for (auto s : states_) {
    ids.insert(make_pair(s, ids.size()+1));
  }  

  os << "reg[31:0] count = 0;" << endl;
  os << "reg[31:0] state = 0;" << endl;
  os << "reg[31:0] i = 0;" << endl;
  os << "reg[31:0] ie = 0;" << endl;
  os << "reg[7:0] char;" << endl;
  os << endl;
  os << "integer itr = 1;" << endl;
  os << "integer s = $fopen(\"share/cascade/test/benchmark/regex/iliad.txt\", \"r\");" << endl;
  os << endl;
  os << "always @(posedge clock.val) begin" << endl;
  os << "  $fscanf(s, \"%c\", char);" << endl;
  os << "  if ($feof(s)) begin" << endl;
  os << "    if (itr == 1) begin" << endl;
  os << "      $write(count);" << endl;
  os << "      $finish(0);" << endl;
  os << "    end else begin" << endl;
  os << "      itr <= itr + 1;" << endl;
  os << "      $rewind(s);" << endl;
  os << "    end" << endl;
  os << "  end else begin" << endl;
  os << "    if (state > 0) begin" << endl;
  os << "      ie <= ie + 1;" << endl;
  os << "    end" << endl;
  os << "    case (state)" << endl;
  os << "      32'd0:" << endl;
  os << "        state <= " << ids[entry_] << ";" << endl;
  for (auto s : states_) {
    os << "      32'd" << ids[s] << ": case(char) " << endl;
    for (auto& t : s->ts) {
      os << "        8'h" << hex << (int)t.first << dec << ": begin" << endl;
      if (is_accept(*t.second.begin())) {
        os << "          //$display(\"Match %d:%d\", i, ie);" << endl;
        os << "          i <= ie + 1;" << endl;
        os << "          count <= count + 1;" << endl;
        os << "          state <= " << ids[entry_] << ";" << endl;
      } else {
        os << "          state <= " << ids[*t.second.begin()] << ";" << endl;
      }
      os << "        end" << endl;
    }
    os << "        default: begin" << endl;
    os << "          i <= ie + 1;" << endl;
    os << "          state <= " << ids[entry_] << ";" << endl;
    os << "        end" << endl;
    os << "      endcase" << endl; 
  }
  os << "      default: begin" << endl;
  os << "        $display(\"Unrecognized state!\");" << endl;
  os << "        $finish;" << endl;
  os << "      end" << endl;
  os << "    endcase" << endl;
  os << "  end" << endl;
  os << "end";
}

void Nfa::State::connect(char c, State* s) {
  ts[c].insert(s);  
}

char Nfa::epsilon() const {
  return 0;
}

Nfa::State* Nfa::get_state() {
  auto s = new State();
  states_.insert(s);
  return s;
}

bool Nfa::is_entry(State* s) const {
  return s == entry_;
}

bool Nfa::is_accept(State* s) const {
  return accepts_.find(s) != accepts_.end();
}

Nfa::PowerState Nfa::epsilon_closure(const PowerState& ps) const {
  auto res = ps;
  for (auto done = false; !done; ) {
    done = true;
    for (auto s : res) {
      auto t = s->ts.find(epsilon());
      if (t == s->ts.end()) {
        continue;
      }
      size_t n = res.size();
      res.insert(t->second.begin(), t->second.end());
      if (res.size() > n) {
        done = false;
      }
    }
  }
  return res;
}

Nfa::PowerState Nfa::power_entry() const {
  PowerState ps;
  ps.insert(entry_);
  return epsilon_closure(ps);
}

bool Nfa::is_power_accept(const PowerState& ps) const {
  for (auto s : ps) {
    if (is_accept(s)) {
      return true;
    }
  }
  return false;
}

Nfa::TransitionSet Nfa::power_transitions(const PowerState& ps) const {
  TransitionSet res;
  for (auto s : ps) {
    for (auto& t : s->ts) {
      if (t.first == epsilon()) {
        continue;
      }
      res.insert(t.first);
    } 
  }
  return res;
}

Nfa::PowerState Nfa::power_transition(const PowerState& ps, char c) const {
  PowerState res;
  for (auto s : ps) {
    auto t = s->ts.find(c);
    if (t != s->ts.end()) {
      res.insert(t->second.begin(), t->second.end());
    }
  }
  return epsilon_closure(res);
} 

Nfa* Nfa::powerset_construction() const {
  auto res = new Nfa();

  map<PowerState, State*> ids;
  stack<PowerState> work_set;

  const auto entry = power_entry();
  ids[entry] = res->entry_;
  work_set.push(entry);

  while (!work_set.empty()) {
    auto ps1 = work_set.top();
    work_set.pop();
    auto s1 = ids[ps1];

    if (is_power_accept(ps1)) {
      res->accepts_.insert(s1);
    }
    for (auto t : power_transitions(ps1)) {
      auto ps2 = power_transition(ps1, t);
      auto itr = ids.find(ps2);

      if (itr != ids.end()) {
        s1->connect(t, itr->second);
      } else {
        auto s2 = res->get_state();
        ids[ps2] = s2;
        s1->connect(t, s2);
        work_set.push(ps2);
      }
    }
  }  

  return res;
}

} // namespace ns
