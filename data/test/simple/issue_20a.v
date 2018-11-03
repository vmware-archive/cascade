reg[7:0] r;
initial begin
  // This index is undefined, but it isn't technically an error.
  // The only thing this test is checking is that we don't crash
  // here.
  r[100] = 1;
  $finish;
end
