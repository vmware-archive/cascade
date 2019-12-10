reg x[1:0];

initial begin
  x[0] = 0;
  $write(x[0]); // Previous write succeeds, prints 0
  x[1000] = 1;
  $write(x[0]); // Previous write evaporates, prints 0
  $write(x[1]); // Previous write evaporates, prints 0
  $finish;
end
