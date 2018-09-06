// This test is a distilled version of the bitcoin_8 benchmark.  However
// compared to pipeline_1.v, the stages perform non-trivial compute with a
// submodule.

module Compute(in, out1, out2);
  input wire[7:0] in;
  output wire[7:0] out1, out2;

  assign out1 = in << 1;
  assign out2 = in + 1;
endmodule

// Pipeline Stage: Adds 1 to input every clock
module Stage(clk, in, out);
  input wire clk;
  input wire[7:0] in;
  output reg[7:0] out;

  wire[7:0] temp1, temp2;
  Compute c1(in, temp1, temp2);

  always @(posedge clk) begin
    out <= temp1 + temp2; 
  end
endmodule

// Pipeline: DEPTH instances of Stage, chained together, plus a final delay
module Pipeline #(parameter DEPTH = 8) (clk, in, out);
  input wire clk;
  input wire[7:0] in;
  output reg[7:0] out;
  
  genvar i;
  for (i = 0; i < DEPTH; i=i+1) begin : STAGES
    wire[7:0] temp;
    if (i == 0) begin
      Stage s(clk, in, temp);
    end else begin
      Stage s(clk, STAGES[i-1].temp, temp);
    end
  end
  
  always @(posedge clk) begin
    out <= STAGES[DEPTH-1].temp * 2; 
  end
endmodule

// Main: Stuff zeros in the pipe and wait until something comes out
localparam[7:0] D = 8;
reg[7:0] in = 8'b0;
wire[7:0] out;

Pipeline #(D) p(clock.val, in, out);

reg[7:0] COUNT = 8'b0;
always @(posedge clock.val) begin
  $write("%d", COUNT);
  COUNT <= COUNT + 1;
  if (out == 160) begin
    $finish;
  end
end
