// This march file represents the smallest possible runtime interface for
// cascade.  It instantiates the root and a single software-backed virtual
// clock.

include data/stdlib/stdlib.v;

(*__target="sw", __loc="runtime"*)
Root root();

(*__target="sw", __loc="runtime"*)                    
Clock clock();
