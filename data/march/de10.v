// This march file corresponds to the DE10 nano SoC.

include data/stdlib/stdlib.v;

(*__target="sw", __loc="runtime"*)
Root root();

(*__target="sw", __loc="runtime"*)                    
Clock clock();

(*__target="de10", __loc="runtime"*)
Pad#(4) pad();

(*__target="de10", __loc="runtime"*)
Led#(8) led();

(*__target="de10", __loc="runtime"*)
Gpio#(8) gpio();
