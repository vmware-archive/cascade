// Self-assignment in a scalar

wire[31:0] x;
assign x[0] = 0;
assign x[1] = x[0] + 1;

initial begin
  $write(x);
  $finish;
end
