// Ordered port connections

module foo(x,y);
  input wire x;
  output wire y;
endmodule

wire x;
wire y;
foo f(x, y);

initial $finish;
