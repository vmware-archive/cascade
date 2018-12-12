// Trace up through a module

module foo();
  initial begin : x 
    y = 1;
  end
endmodule

reg y;
foo f();

initial $finish;
