localparam x = 8'b00001110;
initial begin 
  $write(x[1+:3]);
  $finish;
end
