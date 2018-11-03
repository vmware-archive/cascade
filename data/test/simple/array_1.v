wire[31:0] x[3:0];

genvar i;
for (i = 0; i < 4; i=i+1) begin
  assign x[i] = i;
end

initial begin
  $write(x[0]);
  $write(x[1]);
  $write(x[2]);
  $write(x[3]);
  $finish;
end
