// Check read addr == write addr

reg[3:0] COUNT = 0;
wire[7:0] rd1;

(*__target="sw"*)
Memory#(2,8) mem1(
  .clock(clock.val),
  .wen(1), 
  .raddr1(0), 
  .rdata1(rd1), 
  .waddr(0),
  .wdata(rd1 + 1) 
);

always @(posedge clock.val) begin
  COUNT <= COUNT + 1;
  if (COUNT == 8) begin
    $finish;
  end else begin
    $write("%h", rd1);
  end
end
