stream s = $fopen("data/test/regression/simple/io_1.dat");
reg[31:0] r;
integer count = 1;
always @(posedge clock.val) begin
  $get(s, r);
  if ($feof(s)) begin 
    if (count == 2) begin
      $finish; 
    end else begin
      count <= count + 1;
      $fseek(s, 0);
    end
  end else begin 
    $write(r); 
  end
end
