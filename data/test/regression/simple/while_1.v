integer x = 3;

initial begin 
  while (x) begin
    $write("3");
    x = x - 1;
  end
  $finish;
end
