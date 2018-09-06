localparam x = 1;
if (x) begin
  wire[32:0] temp = 32;
  initial begin
    $write(temp);
    $finish;
  end
end else ;
