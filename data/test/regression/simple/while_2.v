integer x = 0;
integer y = 0;

initial begin 
  while (x < 3) begin
    while (y < 2) begin 
      $write(2*x+y);
      y = y + 1;
    end
    y = 0;
    x = x + 1;
  end
  $finish;
end
