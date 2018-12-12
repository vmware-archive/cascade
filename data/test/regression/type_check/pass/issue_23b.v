// This program should not produce an error.
// At declaration time, we can't know for sure whether X will be
// resolved or not. 

module foo();
  wire[X:0] x;
endmodule

initial $finish;
