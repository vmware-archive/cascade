localparam x = 8;
localparam y = 0;

initial begin
  $write(x == y);
  $write(x == x);
  $finish;
end
