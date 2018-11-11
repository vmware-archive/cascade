// Non-constant in indexed part-select for a register
reg[3:0] r;
reg s = 1;

initial begin
  $display(r[s +: 1]);
  $finish;
end
