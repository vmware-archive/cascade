// This march file corresponds to the software-simulated fpga that comes with
// cascade.

include data/stdlib/stdlib.v;

(*__target="sw", __loc="runtime"*)
Root root();

(*__target="sw", __loc="runtime"*)                    
Clock clock();

(*__target="sw", __loc="/tmp/fpga_socket"*)
Reset reset();

(*__target="sw", __loc="/tmp/fpga_socket"*)
Pad#(4) pad();

(*__target="sw", __loc="/tmp/fpga_socket"*)
Led#(8) led();
