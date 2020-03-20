wire [15:0] x = 16'hDEAD;
wire [15:0] y = 16'hdead;
initial begin
  $display("%h", x);
  $display("%h", y);
  $finish;
end
