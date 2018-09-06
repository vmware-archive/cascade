reg[3:0] x;
reg[3:0] COUNT = 0;

always @(posedge clock.val) begin
  if (x == 0) begin
    x <= 1;
  end else begin
    x[3] <= x[2];
    x[2] <= x[1];
    x[1] <= x[0];
    x[0] <= x[3];
  end

  $write("%d ", x);
  COUNT <= COUNT + 1;
  if (COUNT == 4)
    $finish;
end
