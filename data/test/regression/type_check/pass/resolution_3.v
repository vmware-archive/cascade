// Trace down inside a module through module id

initial begin: foo
  reg x;
end
initial root.foo.x = 1;

initial $finish;
