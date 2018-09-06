localparam x = 'b1;

initial begin
  case (x) 
    1'b0: $write("no");
    1'b1: $write("yes");
  endcase
  $finish;
end
