reg[1:0] x = 2'd3;

initial begin 
  while (x) begin
    $write("3");
    x = x - 2'd1;
  end
  $finish;
end
