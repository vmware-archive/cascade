`ifndef HARNESS_V
`define HARNESS_V

`include "program_logic.v"
`include "uart.v"

module Harness (
  input wire clk,
  input wire ftdi_to_fpga,
  output wire fpga_to_ftdi
);

  wire ready;
  wire enable;
  wire[63:0] rdata;
  wire[31:0] wdata;
    
  Uart#(
    .DIVIDER(25000000/115200),
    .RSIZE(8),
    .WSIZE(4)
  )uart(
    .clk(clk),
    .ready(ready),
    .enable(enable),
    .ftdi_to_fpga(ftdi_to_fpga),
    .rdata(rdata),
    .fpga_to_ftdi(fpga_to_ftdi),
    .wdata(wdata)
  );

  wire read = ready && (rdata[63] == 1'b0);
  wire write = ready && (rdata[63] == 1'b1);
  wire[31:0] address = {1'b0, rdata[62:32]};
  wire[31:0] readdata;
  wire[31:0] writedata = rdata[31:0];
  wire waitrequest;

  program_logic pl (
    .clk(clk),
    .reset(1'b0),
    .s0_address(address),
    .s0_read(read),
    .s0_write(write),
    .s0_readdata(readdata),
    .s0_writedata(writedata),
    .s0_waitrequest(waitrequest)
  );

  assign wdata = read ? readdata : 32'hffffffff;
  assign enable = waitrequest == 1'b0;

endmodule

`endif
