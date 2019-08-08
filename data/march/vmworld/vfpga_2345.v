`ifndef __CASCADE_DATA_MARCH_VMWORLD_FPGA_2345_V
`define __CASCADE_DATA_MARCH_VMWORLD_FPGA_2345_V

`include "data/stdlib/stdlib.v"

Root root();

Clock clock();

module VPad(
  output wire[15:0] val
);
  (*__target="de10", __loc="192.168.8.2:8800"*)
  Pad#(4) p1();
  (*__target="de10", __loc="192.168.8.3:8800"*)
  Pad#(4) p2();
  (*__target="de10", __loc="192.168.8.4:8800"*)
  Pad#(4) p3();
  (*__target="de10", __loc="192.168.8.5:8800"*)
  Pad#(4) p4();

  assign val = {p4.val, p3.val, p2.val, p1.val};
endmodule
VPad pad();

module VLed(
  input wire[31:0] val
);
  (*__target="de10", __loc="192.168.8.2:8800"*)
  Led#(8) l1();
  (*__target="de10", __loc="192.168.8.3:8800"*)
  Led#(8) l2();
  (*__target="de10", __loc="192.168.8.4:8800"*)
  Led#(8) l3();
  (*__target="de10", __loc="192.168.8.5:8800"*)
  Led#(8) l4();

  assign l1.val = val[0+:8];
  assign l2.val = val[8+:8];
  assign l3.val = val[16+:8];
  assign l4.val = val[24+:8];
endmodule
VLed led();

`endif
