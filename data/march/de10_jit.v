`ifndef __CASCADE_MARCH_DE10_JIT_V
`define __CASCADE_MARCH_DE10_JIT_V

`include "data/stdlib/stdlib.v"

(*__target="sw;de10"*)
Root root();

Clock clock();

(*__target="de10"*)
Pad#(4) pad();

(*__target="de10"*)
Led#(8) led();

(*__target="de10"*)
Gpio#(8) gpio();

`endif
