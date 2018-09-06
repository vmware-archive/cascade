#include <iostream>

using namespace std;

int main(int argc, char** argv) {
  if (argc != 1) {
    cout << "Usage: cat <file.txt> | " << argv[0] << endl;
    return 1;
  }

  cout << hex;

  char c = ' ';
  while (cin.get(c)) {  
    cout << (int)c << " ";
  }

  return 0;
}
