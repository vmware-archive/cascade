`ifndef __SHARE_CASCADE_MARCH_DE10_V
`define __SHARE_CASCADE_MARCH_DE10_V

`include "share/cascade/stdlib/stdlib.v"

(*__target="sw;verilator32;de10"*)
Root root();

Clock clock();

(*__target="de10"*)
Pad#(4) pad();

(*__target="de10"*)
Led#(8) led();

(*__target="de10"*)
Gpio#(8) gpio();

`endif
