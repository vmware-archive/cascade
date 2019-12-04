module foo();
  parameter x = 1;
  if (x) begin
    wire y = 0;
  end else begin
    wire z = 0;
  end
endmodule

foo f();

initial $finish;
