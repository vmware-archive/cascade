// Undeclared explicit port

module foo(x);
  input wire x;
endmodule

wire x;
foo f(.y(x));

initial $finish;
