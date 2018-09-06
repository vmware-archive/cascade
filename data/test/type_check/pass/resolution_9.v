// Trace up through multiple modules

module foo();
  initial begin : x
    y = 1;
  end
endmodule

module bar();
  foo f();
endmodule

reg y;
bar b();

initial $finish;
