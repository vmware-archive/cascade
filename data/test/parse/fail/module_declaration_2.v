module foo(
  input wire x,y,z,     
  output wire a=10,b,c  // FAIL: Can't assign a value to an output wire
);
endmodule
