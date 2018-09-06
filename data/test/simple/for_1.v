reg[2:0] x;
initial begin
  for (x = 3'd0; x < 3'd3; x = x + 1) begin
    $write("3");
  end
  $finish;
end
