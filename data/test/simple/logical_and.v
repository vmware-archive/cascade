localparam x = 8;
localparam y = 3;
localparam z = 0;

initial begin
  $write(x&&z);
  $write(x&&y);
  $write(x&&x);
  $finish;
end
