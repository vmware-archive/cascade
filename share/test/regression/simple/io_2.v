integer s = $fopen("share/test/regression/simple/io_2.dat", "r");
reg [15:0] r1 = 0;
reg signed[31:0] r2 = 0;
reg [31:0] r3 = 0;
reg [39:0] r4 = 0;
real r5 = 0;

initial begin
  $fscanf(s, "%d %d %c %s %f", r1, r2, r3, r4, r5);
  $write("%h %d %c %s %.2f", r1, r2, r3, r4, r5);
  $finish;
end
