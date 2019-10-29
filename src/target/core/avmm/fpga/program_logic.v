module program_logic(
  input wire clk,
  input wire reset,

  input wire[15:0]  s0_address,
  input wire        s0_read,
  input wire        s0_write,

  output reg [31:0] s0_readdata,
  input  wire[31:0] s0_writedata,

  output reg        s0_waitrequest
);

endmodule
