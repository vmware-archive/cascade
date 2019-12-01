integer s = $fopen("share/test/regression/simple/io_4.dat", "r");
reg[15:0] r1 = 0;
reg[15:0] r2 = 0;
reg signed[15:0] r3 = 0;

initial begin
  $fscanf(s, "%f %d %d", r1, r2, r3);
  $write("%d %d %d", r1, r2, r3);
  $finish;
end
