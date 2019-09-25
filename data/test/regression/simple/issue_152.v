reg[54:0] subtrahend = 55'h7bcb43d769f764; 
reg[10:0] exponent_diff = 11'h056;

initial begin
  $display("%h >> %d = %h", subtrahend, exponent_diff, (subtrahend >> exponent_diff));
  $finish;
end
