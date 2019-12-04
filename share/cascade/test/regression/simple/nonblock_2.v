// This test is a distilation of what makes bitcoin_7.v difficult.
//
// The trouble is a race condition between a constant value (x) which
// needs to pass through two modules (latch and shift) before its value
// is recorded by the always block in latch, and clock.val which triggers
// that always block.

// The solution is to make sure that elaborated code isn't just SCHEDULED
// between clock-ticks, but that's also step'ed so that whatever constant
// values are introduced are propagated through the entire circuit. 

// You can think of wires with constant values as having the same semantics
// as initial blocks. Their values are computed and propagated immediately
// IN BETWEEN clock ticks, so that when control resumes, everything is back
// in a consistent state.

module Shift(x,y);
  input wire[1:0] x;
  output wire[1:0] y;

  assign y = x << 1;
endmodule

module Latch(c,x,y);
  input wire c;
  input wire[1:0] x;
  output reg[1:0] y;

  wire [1:0] temp;
  Shift shift(x, temp);

  always @(posedge c) begin
    y <= x | temp;
  end
endmodule

wire[1:0] x = 2'b1;
wire[1:0] y; 

Latch l(clock.val, x, y);

always @(posedge clock.val) begin
  if (y) begin
    $write(y);
    $finish;
  end
end
