// This test is a distilled version of the bitcoin_8 benchmark.
// This is larger instantiation. It uses 8 stages.

// Pipeline Stage: Adds 1 to input every clock
module Stage(clk, in, out);
  input wire clk;
  input wire[7:0] in;
  output reg[7:0] out;

  always @(posedge clk) begin
    out <= in + 8'b1;
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
  if ((COUNT == 16) || (out == (D*2))) begin
    $finish;
  end
end
