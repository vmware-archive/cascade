// This march file corresponds to the DE10 nano SoC.

include data/stdlib/stdlib.v;

Root root();

Clock clock();

(*__target="de10"*)
Pad#(4) pad();

(*__target="de10"*)
Led#(8) led();

(*__target="de10"*)
Gpio#(8) gpio();
