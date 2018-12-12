localparam[3:0] x = 4'b0011;
localparam[3:0] y = 4'b0101;

initial begin
  $write(x | y);
  $finish;
end

