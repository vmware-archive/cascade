// Trace up and down inside a module

initial begin
  begin : foo
    integer x;
  end
  begin : bar
    foo.x = 1;
  end
end

initial $finish;
