integer i = 0;
integer j = 0;

initial begin
  for (i = 0; i < 3; i = i+1) begin
    for (j = i; j < 3; j = j+1) begin
      $write(3*i+j);
    end
  end
  $finish;
end
