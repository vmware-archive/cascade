// This program should produce an error.
// At declaration time, we can't know for sure whether X will be
// resolved or not. But at instantiation time, we'll fail.

module foo();
  wire[31:0] x;
  assign x[X:0] = 1;
endmodule

foo f();
