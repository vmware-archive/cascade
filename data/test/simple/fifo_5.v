wire[4:0] rdata;
wire empty;

reg[31:0] sum = 0;

(*__target="sw", __file="data/test/simple/fifo_4.data", __count=2*)
Fifo#(1,4) fifo(
  .clock(clock.val),
  .rreq(!empty),
  .rdata(rdata),
  .empty(empty)
);

always @(posedge clock.val) begin
  if (empty) begin
    $write(sum + rdata);
    $finish;
  end else begin 
    sum <= sum + rdata;
  end
end
