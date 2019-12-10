integer i = 0;
initial begin
  for (i = 0; i < 3; i=i+1) begin
    $write(i);
  end
  for (i = 0; i < 3; i=i+1) begin
    $write(i);
  end
  $finish;
end
