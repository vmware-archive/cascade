integer s = $fopen("data/test/regression/simple/io_1.dat");
reg[31:0] r;
integer count = 1;
always @(posedge clock.val) begin
  $fread(s, r);
  if ($feof(s)) begin 
    if (count == 2) begin
      $finish; 
    end else begin
      count <= count + 1;
      $rewind(s);
    end
  end else begin 
    $write(r); 
  end
end
