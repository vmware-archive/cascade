// This march file is intended for debugging purposes only. It enables jit
// functionality but uses the same backend for both passes. The only value of
// using this march file in testing whether jit compilation works correctly.

`include "data/stdlib/stdlib.v"

(*__target="sw;sw"*)
Root root();

Clock clock();
