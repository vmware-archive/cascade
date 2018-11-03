reg[3:0] COUNT = 0;
wire[3:0] rdata;
wire empty, full;

Fifo#(4,3) fifo(
  .clock(clock.val),
  .wreq(COUNT < 4),
  .wdata(COUNT+1),
  .rreq(COUNT >= 4),
  .rdata(rdata),
  .empty(empty),
  .full(full)
);

always @(posedge clock.val) begin
  COUNT <= COUNT + 1;
  if (COUNT == 9) begin
    $finish;
  end 
  // On COUNT 0-3 we're pushing, so by COUNT 4, we should see full fifo
  if (COUNT < 5) begin
    $write("%h%h", empty, full);
  end 
  // Beginning from COUNT 4, we're popping, so we should start to see values
  else begin
    $write("%h%h%h", rdata, empty, full);
  end
end
