`ifndef __CASCADE_DATA_MARCH_FPGA_5_V
`define __CASCADE_DATA_MARCH_FPGA_5_V

`include "data/stdlib/stdlib.v"

Root root();

Clock clock();

(*__target="de10", __loc="192.168.8.5:8800"*)
Pad#(4) pad();

(*__target="de10", __loc="192.168.8.5:8800"*)
Led#(8) led();

`endif
