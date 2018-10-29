module foo(
  input wire x, 
  input wire[1:0] y
);
endmodule

wire r;
wire [15:0] s;

// This instantiation should succeed: 
// r is a perfect match 
// but s doesn't fit  evenly when it's divided by the number of instantiations
foo f[1:0](r, s);
