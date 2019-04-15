localparam x = 1;
localparam y = 2;
localparam z = 3;

initial begin 
  repeat(x + y + z) $write("6");
  $finish;
end
