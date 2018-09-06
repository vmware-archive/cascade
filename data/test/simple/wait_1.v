reg x = 'b0;

initial begin
  $write("Hello ");
  wait(x) begin 
    $write("World");
    $finish;
  end
end

initial begin
  x = 'b1;
end
