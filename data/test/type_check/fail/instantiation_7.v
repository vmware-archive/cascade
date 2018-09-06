module foo(x,y);
  input wire x;
  output wire y;
endmodule

foo f(1+2, 1+2);
initial $finish;
