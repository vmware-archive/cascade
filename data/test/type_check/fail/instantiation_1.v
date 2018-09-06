// More arguments than declared ports

module foo();
endmodule

wire x;
foo f(x);

initial $finish;
