// This march file is intended for debugging purposes only. It exercises remote
// functionality for generic program logic.

include data/stdlib/stdlib.v;

(*__target="sw", __loc="/tmp/fpga_socket"*)
Root root();

(*__target="sw", __loc="runtime"*)                    
Clock clock();
