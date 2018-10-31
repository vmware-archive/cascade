reg r[1:0];
integer i[1:0];
// Out of bounds accesses are allowed for registers and integers
initial begin 
  r[100] = i[1000];
  $finish;
end
