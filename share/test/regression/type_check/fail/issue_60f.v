wire x;
initial begin
  $write("Too few args %d %d %d", x);
  $finish;
end
