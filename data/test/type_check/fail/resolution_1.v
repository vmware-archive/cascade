// Unresolvable instance select
genvar i;
for (i=0;i<10;i=i+1) begin : FOO
  reg r;
end

initial $display(FOO[27].r);
