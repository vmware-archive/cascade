real r3 = 32'h00000000 | 32'h10101010;
reg[31:0] r1 = 269488144.00;
reg[63:0] r2;
initial begin
  r2[31:0] = r3;
  $write(r1);
  $write(r2);
  $finish;
end
