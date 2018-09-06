localparam x = {2 {32'h ffffffff}};

initial begin 
  $write("%h", x);
  $finish;
end
