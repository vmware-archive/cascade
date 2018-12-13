reg idx = 0;
reg r[1:0];

always @(posedge clock.val) begin
  if (r[0] || r[1]) begin
    $write("%d%d", r[0], r[1]);
    $finish;
  end

  idx <= idx + 1;
  // idx must be latched on BOTH sides of this assignment.  When this test
  // finishes, we expect r[0] to be equal to 1.  If we don't latch idx on the
  // lhs, r[1] will be set to 1 instead.
  r[idx] <= r[idx] + 1;

end
