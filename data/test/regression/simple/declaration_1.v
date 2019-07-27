reg[64:33] r = 32'h12345678;
initial begin 
  $write(r[36:33]);
  $finish;
end
