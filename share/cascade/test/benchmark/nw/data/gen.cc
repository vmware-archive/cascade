#include <cstdlib>
#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char** argv) {
  ifstream ifs(argv[1]);

  string ignore;
  size_t num_inputs;
  size_t length;

  getline(ifs, ignore);
  getline(ifs, ignore);
  getline(ifs, ignore);
  ifs >> ignore >> ignore >> ignore >> num_inputs; getline(ifs, ignore);
  getline(ifs, ignore);
  ifs >> ignore >> ignore >> ignore >> length >> ignore; getline(ifs, ignore);

  for (size_t i = 0; i < num_inputs; ++i) {
    for (size_t j = 0; j < length; ++j) {
      const auto pair = rand() % 16;
      cout << (pair < 10 ? (char)('0'+pair) : (char)('a'+pair-10));
    }    
    cout << endl;
  }

  return 0;
}
