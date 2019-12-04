localparam[3:0] x = 4'b0011;

initial begin
  $write(~x);
  $finish;
end
