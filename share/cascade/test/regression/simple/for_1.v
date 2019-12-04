integer x = 0;
initial begin
  for (x = 0; x < 3; x = x+1) begin
    $write("3");
  end
  $finish;
end
