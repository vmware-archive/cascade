// This used to be a standard library test. But now that we can implement
// Memories using arrays, this has become an array test.

module Mem#(
  parameter ADDR_SIZE = 4,
  parameter BYTE_SIZE = 8
)(
  input  wire clock,
  input  wire wen,
  input  wire[ADDR_SIZE-1:0] raddr1,
  output wire[BYTE_SIZE-1:0] rdata1,
  input  wire[ADDR_SIZE-1:0] raddr2,
  output wire[BYTE_SIZE-1:0] rdata2,
  input  wire[ADDR_SIZE-1:0] waddr,
  input  wire[BYTE_SIZE-1:0] wdata
);
  reg[BYTE_SIZE-1:0] mem[ADDR_SIZE-1:0];
  assign rdata1 = mem[raddr1];
  assign rdata2 = mem[raddr2];
  always @(posedge clock)
    if (wen) 
      mem[waddr] <= wdata;
endmodule

reg[3:0] COUNT = 0;
wire[1:0] raddr = COUNT;
wire[1:0] waddr = COUNT+1;
wire[2:0] rd1, rd2;

Mem#(4,3) mem1(
  .clock(clock.val),
  .wen(1), 
  .raddr1(raddr), 
  .rdata1(rd1), 
  .raddr2(raddr), 
  .rdata2(rd2),
  .waddr(waddr),
  .wdata(COUNT+1) 
);

always @(posedge clock.val) begin
  COUNT <= COUNT + 1;
  if (COUNT == 8) begin
    $finish;
  end else begin
    $write("%h%h", rd1, rd2);
  end
end
