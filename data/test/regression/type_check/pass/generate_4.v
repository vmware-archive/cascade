module foo();
  parameter IDX = 2;

  genvar i;
  for (i = 0; i < 8; i = i + 2) begin : b
    wire[3:0] x = 1;
  end

  assign b[0*10].x = b[IDX].x;
  assign b[IDX * 2].x = b[IDX + IDX + IDX].x;
endmodule

foo f();
initial $finish;
