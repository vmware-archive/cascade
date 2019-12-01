// This test checks whether cascade treats genvars as integers during compilation

genvar i;
for (i = -1; (i < 0) && (i >= -1); i=i-1) begin : FOO
  reg[31:0] r = 10;
end

initial begin
  $write(FOO[-1].r);
  $finish;
end
