module foo();
  initial begin
    w = 10;
    x = #1 20;
    y <= 30;
    z <= #1 40;
  end
endmodule
