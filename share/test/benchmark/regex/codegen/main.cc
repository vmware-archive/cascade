#include <iostream>
#include "parser.h"

using namespace ns;
using namespace std;

int main (int argc, char** argv) {
  if (argc != 1) {
    cerr << "Usage: cat <input> | " << argv[0] << endl;
    return 1;
  }

  Parser p;
  auto nfa = p.parse(cin);
  if (p.error()) {
    cout << p.why() << endl;
    return 2;
  } 

  nfa->make_deterministic();
  nfa->to_verilog(cout);

  delete nfa;
  return 0;
}
