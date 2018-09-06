wire [7:0] x = {2+2 {2'b10}};
initial begin 
  $write(x);
  $finish;
end
