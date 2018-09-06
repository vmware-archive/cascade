// Trace up inside a module

reg x;
initial begin : foo
  x = 1;
end

initial $finish;
