// This program should produce an error.
// At declaration time, we can't know for sure whether X will be
// resolved or not. But at instantiation time, we'll fail.

module foo();
  wire x = X;
endmodule

foo f();
