reg[7:0] x = 0;

initial begin
  $write(x); // Prints 0
  x[1000] = 1;
  $write(x); // Previous write should evaporate. Prints 0.
  x[1000:1] = -1;
  $write(x); // Previous write should partially succeed. Prints 254
  $finish;
end
