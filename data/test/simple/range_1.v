localparam x = 8'b00001110;
initial begin
  $write(x[4:1]);
  $finish;
end
