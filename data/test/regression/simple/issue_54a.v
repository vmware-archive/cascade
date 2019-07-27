// Self-assignment in an array.

wire[31:0] x[1:0];
assign x[0] = 0;
assign x[1] = x[0] + 1;

initial begin
  $write(x[1]);
  $finish;
end
