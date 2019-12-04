localparam x = 'b1;

initial begin
  case (x) 
    1'b0: $write("no");
    1'b1: begin 
      $write("1");
      $write("2");
    end
  endcase
  $write("3");
  $finish;
end
