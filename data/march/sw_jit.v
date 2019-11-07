`ifndef __CASCADE_DATA_MARCH_SW_JIT_V
`define __CASCADE_DATA_MARCH_SW_JIT_V

`include "data/stdlib/stdlib.v"

(*__target="sw;sw", __loc="local;/tmp/fpga_socket"*)
Root root();

Clock clock();

(*__loc="/tmp/fpga_socket"*)
Reset reset();

(*__loc="/tmp/fpga_socket"*)
Pad#(4) pad();

(*__loc="/tmp/fpga_socket"*)
Led#(8) led();

`endif

