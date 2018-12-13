// This program should not produce an error.
// At declaration time, we can't know for sure whether X will be
// resolved or not. 

module foo();
  wire[31:0] x;
  assign x[X:0] = 1;
endmodule

initial $finish;
