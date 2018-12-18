wire x;
initial begin
  $write("Too many args %d", x, x, x);
  $finish;
end
