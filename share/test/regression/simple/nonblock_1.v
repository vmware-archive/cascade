reg x = 0;
always @(posedge clock.val) begin
  x <= 1;
  if (x) begin
    $write("1"); 
    $finish;
  end else
    $write("0");
end
