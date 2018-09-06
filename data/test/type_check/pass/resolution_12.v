// Address a module-local variable from the root

module foo();
  wire x;

  assign root.f.x = 1;
endmodule

foo f();

initial $finish;
