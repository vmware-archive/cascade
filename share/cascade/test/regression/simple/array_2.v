reg[7:0] r = 8'hff;
wire[7:0] w1[7:0];
assign w1[7] = r;

wire[7:0] w2[7:0][7:0];
assign w2[7][7] = w1[7];

initial begin
  // Should print 0 255
  $write("%d%d", w2[0][0], w2[7][7]);
  $finish;
end
