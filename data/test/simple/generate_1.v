reg[3:0] COUNT = 0;
always @(posedge clock.val) begin
  COUNT <= COUNT + 1;
end

genvar i;
for (i = 0; i < 8; i = i + 1) begin
  always @(posedge clock.val) if (COUNT == i) $write(i);
end

always @(posedge clock.val) if (COUNT == 8) $finish;
