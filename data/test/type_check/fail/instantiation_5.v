// Duplicate instantiations 

module foo();
endmodule

foo f();
foo f();

initial $finish;
