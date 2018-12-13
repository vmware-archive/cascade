wire [3:0] x = {2'b10, 2'b10};
wire [7:0] y = {x,x}; 
initial begin
  $write(y);
  $finish;
end
