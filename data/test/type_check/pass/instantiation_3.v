// Ordered parameter connections

module foo();
  parameter x = 0;
  parameter y = 1;
endmodule

foo #(1,2) f();

initial $finish;
