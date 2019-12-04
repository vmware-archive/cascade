// Reads and writes to bit and part selects which are out of declared bounds
// are undefined. We can emit a warning, but this isn't an error.

wire[3:0] w;
assign w[5:2] = 1;

reg[3:0] r;
assign w[15] = r[27];

initial begin
  $finish;
end
