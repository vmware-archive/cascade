// Non-constant value in instance select
genvar i;
for (i = 0; i < 10; i=i+1) begin : FOO
  reg r;
end

reg s = 1;
initial $display(FOO[s].r);
