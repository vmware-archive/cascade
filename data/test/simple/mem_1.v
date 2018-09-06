reg[3:0] COUNT = 0;
wire[2:0] rd1, rd2;

(*__target="sw"*)
Memory#(2,3) mem1(
  .clock(clock.val),
  .wen(1), 
  .raddr1(COUNT), 
  .rdata1(rd1), 
  .raddr2(COUNT), 
  .rdata2(rd2),
  .waddr(COUNT+1),
  .wdata(COUNT+1) 
);

always @(posedge clock.val) begin
  COUNT <= COUNT + 1;
  if (COUNT == 8) begin
    $finish;
  end else begin
    $write("%h%h", rd1, rd2);
  end
end
