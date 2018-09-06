initial $write("once");

reg[63:0] count = 0;
always @(posedge clock.val) begin
  count <= count + 1;
  if (count == 1024) 
    $finish;
end
