module foo(x,y);
  input wire x;
  output wire y;
endmodule

foo f(.x(1+2), .y(1+2));
initial $finish;
