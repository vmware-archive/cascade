// This instantiation should not succeed: 
// r is a perfect match 
// but s doesn't fit evenly when it's divided by the number of instantiations

module foo(
  input wire x, 
  input wire[1:0] y
);
endmodule

wire r;
wire [15:0] s;

foo f[1:0](r, s);
