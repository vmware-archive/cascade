genvar i;
always @(posedge clock.val) begin
  for (i = 0; i < 10; i = i + 1) begin end
end
