module foo(x);
  input wire x;
endmodule

foo f(clock.val);
initial $finish;
