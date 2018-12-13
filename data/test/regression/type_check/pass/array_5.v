// Non-const reference into net array on rhs
reg r = 1;
wire[3:0] w[3:0];

initial begin
  $display(w[r][r +: 1]);
  $finish;
end
