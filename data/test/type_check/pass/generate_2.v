module foo();
  genvar i;
  for (i = 0; i < 4; i = i + 1) begin
    initial $display(i);
  end
endmodule

foo f();

initial $finish;
