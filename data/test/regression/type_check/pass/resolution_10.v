// Trace up and down through multiple modules

module foo();
  initial begin : x
    reg y;
  end
endmodule

module bar();
  foo f();
  initial begin : y
    b.f.x.y = 1;
  end
endmodule

bar b();

initial $finish;
