// Trace up and down inside a module through module id

initial begin
  begin : foo
    integer x;
  end
  begin : bar
    root.foo.x = 1;
  end
end

initial $finish;
