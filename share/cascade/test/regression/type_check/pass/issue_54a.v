// *not* a zero-time loop
wire x[31:0];
assign x[1] = x[0];
initial $finish;
