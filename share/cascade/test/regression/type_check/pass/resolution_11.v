// Trace up and down through multiple modules through module id

module foo();
  initial begin : x
    reg y;
  end
endmodule

module bar();
  foo f();
  initial begin : y
    root.b.f.x.y = 1;
  end
endmodule

bar b();

initial $finish;
