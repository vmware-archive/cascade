genvar i;
for (i = 0; i < 10; i = i + 1) begin : FOO
  wire x;
end
initial $display(FOO[Q].x);

