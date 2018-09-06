localparam x = 8;
localparam y = 5;

initial begin
  $write(x%y);
  $finish;
end
