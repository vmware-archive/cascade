localparam x = 'b1;

initial begin
  case (x) 
    1'b0:    $write("no");
    default: $write("yes");
  endcase
  $finish;
end
