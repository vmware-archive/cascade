`ifndef HARNESS_V
`define HARNESS_V

`include "program_logic.v"
`include "uart.v"

module Harness #(
  parameter ADDR_SIZE = 4,
  parameter VAL_SIZE = 4
)(
  input wire clk,
  input wire ftdi_to_fpga,
  output wire fpga_to_ftdi
);

  wire ready;
  wire enable;
  wire[8*(ADDR_SIZE+VAL_SIZE)-1:0] rdata;
  wire[8*VAL_SIZE-1:0] wdata;
    
  Uart#(
    .DIVIDER(25000000/115200),
    .RSIZE(ADDR_SIZE+VAL_SIZE),
    .WSIZE(VAL_SIZE)
  )uart(
    .clk(clk),
    .ready(ready),
    .enable(enable),
    .ftdi_to_fpga(ftdi_to_fpga),
    .rdata(rdata),
    .fpga_to_ftdi(fpga_to_ftdi),
    .wdata(wdata)
  );

  wire read = ready && (rdata[8*(ADDR_SIZE+VAL_SIZE)-1] == 1'b0);
  wire write = ready && (rdata[8*(ADDR_SIZE+VAL_SIZE)-1] == 1'b1);
  wire[8*ADDR_SIZE-1:0] address = {1'b0, rdata[8*(ADDR_SIZE+VAL_SIZE)-2:8*VAL_SIZE]};
  wire[8*VAL_SIZE-1:0] readdata;
  wire[8*VAL_SIZE-1:0] writedata = rdata[8*VAL_SIZE-1:0];
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

  assign wdata = readdata;
  assign enable = waitrequest == 1'b0;

endmodule

`endif
