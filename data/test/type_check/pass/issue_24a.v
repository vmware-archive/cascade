module foo(
  input wire x, 
  input wire[1:0] y
);
endmodule

wire r;
wire [3:0] s;

// This instantiation should succeed: 
// r is a perfect match 
// s fits evenly when it's divided by the number of instantiations
foo f[1:0](r, s);

initial $finish;
