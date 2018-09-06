// This test is identical to generate_1.v, only the iteration range starts at 1
// and doesn't increment one at a time.

reg[3:0] COUNT = 0;
always @(posedge clock.val) begin
  COUNT <= COUNT + 1;
end

genvar i;
for (i = 1; i < 8; i = i + 2) begin
  always @(posedge clock.val) if (COUNT == i) $write(i);
end

always @(posedge clock.val) if (COUNT == 8) $finish;
