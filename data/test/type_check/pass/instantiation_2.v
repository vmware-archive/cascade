// Named port connections

module foo(x,y);
  input wire x;
  output wire y;
endmodule

wire x;
wire y;
foo f(.y(y), .x(x));

initial $finish;
