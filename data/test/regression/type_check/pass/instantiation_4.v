// Named parameter connections

module foo();
  parameter x = 0;
  parameter y = 1;
endmodule

foo #(.y(2), .x(1)) f();

initial $finish;
