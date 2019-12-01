module foo();
  parameter x = 0;
  initial begin 
    $write(x);
    $finish;
  end
endmodule

foo#(.x(1)) f();
