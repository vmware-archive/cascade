// Well-formed hierarchical identifier

genvar i;
genvar j;
for (i=10;i<11;i=i+1) begin : x
  if (1) begin : y
    for (j=30;j<31;j=j+1) begin : z
      wire val;
    end
  end
end

assign x[10].y.z[30].val = 1;
