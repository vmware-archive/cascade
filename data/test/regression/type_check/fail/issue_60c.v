wire x;
initial begin
  $display("Too many args %d", x, x, x);
  $finish;
end
