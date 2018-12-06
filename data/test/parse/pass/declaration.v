module foo();
  wire x1;
  wire x2,x3;
  wire x4 = x1&x2;

  reg r1;
  reg r2, r3 = 27;

  integer i1;
  integer i2, i3 = 15;
endmodule
