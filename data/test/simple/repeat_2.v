localparam x = 3'd1;
localparam y = 3'd2;
localparam z = 3'd3;

initial begin 
  repeat(x + y + z) $write("6");
  $finish;
end
