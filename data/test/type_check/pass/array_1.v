// Reads and writes to array indices which are out of declared bounds
// are undefined. We can emit a warning, but this isn't an error.

reg r[1:0];
integer i[1:0];

initial begin 
  r[100] = i[1000];
  $finish;
end

wire w[3:0];
assign w[5] = 1;
