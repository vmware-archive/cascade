// This march file represents the smallest possible runtime interface for
// cascade.  It instantiates the root and a single software-backed virtual
// clock.

`include "data/stdlib/stdlib.v"

Root root();

Clock clock();
