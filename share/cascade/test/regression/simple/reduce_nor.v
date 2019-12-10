localparam[3:0] x = 4'b0101;
localparam[3:0] y = 4'b0000;

initial begin
  $write(~|x);
  $write(~|y);
  $finish;
end

