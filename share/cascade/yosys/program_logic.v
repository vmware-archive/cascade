module program_logic(
  input wire clk,
  input wire reset,

  input wire[31:0] s0_address,
  input wire s0_read,
  input wire s0_write,

  output wire[31:0] s0_readdata,
  input  wire[31:0] s0_writedata,

  output wire s0_waitrequest
);

  reg[31:0] data[127:0];
  always @(posedge clk) begin
    if (s0_write)
      data[s0_address] <= s0_writedata;
  end
  assign s0_readdata = data[s0_address] << 1;

  reg[9:0] count = 0;
  always @(posedge clk) begin
    count <= count + 1;
  end

  assign s0_waitrequest = (s0_read) && (count > 0);

endmodule
