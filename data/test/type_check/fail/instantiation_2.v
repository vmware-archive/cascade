// Duplicate explicit port connections

module foo(x);
  input wire x;
  input wire y;
  input wire z;
endmodule

wire x;
foo f(.x(x), .x(x));

initial $finish;
