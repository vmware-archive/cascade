// Undeclared explicit port

module foo(x);
  input wire x;
endmodule

module bar();
  wire x;
  foo f(.y(x));
endmodule

initial $finish;
