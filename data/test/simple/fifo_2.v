reg[1:0] COUNT = 0;
wire[3:0] rdata;
wire empty, full;

(*__target="sw"*)
Fifo#(1,3) fifo(
  .clock(clock.val),
  .wreq(COUNT < 1),
  .wdata(COUNT+1),
  .rreq(COUNT >= 1),
  .rdata(rdata),
  .empty(empty),
  .full(full)
);

always @(posedge clock.val) begin
  COUNT <= COUNT + 1;
  if (COUNT == 3) begin
    $finish;
  end 
  // On COUNT 0 we're pushing, so by COUNT 1, we should see full fifo
  if (COUNT < 2) begin
    $write("%h%h", empty, full);
  end 
  // Beginning from COUNT 1, we're popping, so we should start to see values
  else begin
    $write("%h%h%h", rdata, empty, full);
  end
end
