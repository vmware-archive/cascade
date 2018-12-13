// Trace down through multiple modules

module foo();
  reg x = 5;
endmodule

module bar();
  foo f();
endmodule

bar b();
wire q = b.f.x;

initial $finish;
