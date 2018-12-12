localparam x = 8;
localparam y = 0;

initial begin
  $write(!x);
  $write(!y);
  $finish;
end

