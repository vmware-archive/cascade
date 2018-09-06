#include <fstream>
#include <iostream>
#include <sstream>
#include "parser.h"

using namespace ns;
using namespace std;

int main (int argc, char** argv) {
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " <input.m>" << endl;
    return 1;
  }
  ifstream ifs(argv[1]);
  if (!ifs.is_open()) {
    cerr << "Unable to open input file " << argv[1] << endl;
    return 2;
  }

  Parser p;
  p.parse(ifs, cout);

  if (p.error()) {
    cout << p.why() << endl;
    return 3;
  } 

  return 0;
}

