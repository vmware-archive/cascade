`ifndef __CASCADE_DATA_MARCH_VERILATOR_JIT_V
`define __CASCADE_DATA_MARCH_VERILATOR_JIT_V

`include "data/stdlib/stdlib.v"

(*__target="sw;verilator"*)
Root root();

Clock clock();

`endif
