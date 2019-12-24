`include "harness.v"

module Root (
  input clk_25mhz,
  input ftdi_txd,
  output ftdi_rxd,
	output[7:0] led
);   
  
  Harness#(
    .ADDR_SIZE(4),
    .VAL_SIZE(4)    
  ) harness (
    .clk(clk_25mhz),
    .ftdi_to_fpga(ftdi_txd),
    .fpga_to_ftdi(ftdi_rxd)
  );

  assign led = 0;

endmodule
