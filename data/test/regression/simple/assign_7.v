localparam[7:0] led = 8'b10101010;

initial begin
  $write(led);
  $finish;
end
