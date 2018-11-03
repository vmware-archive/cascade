// Non-constant values in bit-select
reg[7:0] idx = 0;
reg[7:0] r;

initial r[idx+1:idx] = 1;
