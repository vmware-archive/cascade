wire x;
initial begin
  $display("Too few args %d %d %d", x);
  $finish;
end
