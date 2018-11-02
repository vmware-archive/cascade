// A slightly more involved version of array_3.v. This one uses
// non-constant expressions for bit-slicing.

reg s = 0;
reg[7:0] idx = 0;
reg[7:0] r[1:0];

always @(posedge clock.val) begin
  if (r[0] || r[1]) begin
    $write("%d%d", r[0], r[1]);
    $finish;
  end

  idx <= idx + 1;
  s <= s + 1;

  r[idx][s+:4] <= r[idx][s+:4] + 4'hf;
  r[idx][(s+4)+:4] <= r[idx][(s+4)+:4] + 4'hf;

end
