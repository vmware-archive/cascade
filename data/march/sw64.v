`ifndef __CASCADE_DATA_MARCH_SW64_V
`define __CASCADE_DATA_MARCH_SW64_V

`include "data/stdlib/stdlib.v"

(*__target="sw;verilator64"*)
Root root();

Clock clock();

(*__loc="/tmp/fpga_socket"*)
Reset reset();

(*__loc="/tmp/fpga_socket"*)
Pad#(4) pad();

(*__loc="/tmp/fpga_socket"*)
Led#(8) led();

`endif
