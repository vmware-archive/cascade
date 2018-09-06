// Trace down into a module

module foo();
  initial begin : x 
    reg y;
  end
endmodule

foo f();
wire q = f.x.y;

initial $finish;
