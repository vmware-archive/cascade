// Trace down inside a module

initial begin: foo
  reg x;
end

initial foo.x = 1;

initial $finish;
