// This march file is intended for debugging purposes only. It enables jit
// functionality but uses the same backend for both passes. The only value of
// using this march file in testing whether jit compilation works correctly.

include data/stdlib/stdlib.v;

(*__target="sw", __target2="sw", __loc="runtime"*)
Root root();

(*__target="sw", __loc="runtime"*)                    
Clock clock();
