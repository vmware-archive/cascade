// This used to be a standard library test. But now that we can implement
// Fifos using arrays, this has become an array test.

module MyFifo#(
  parameter LOG_DEPTH = 3,
  parameter BYTE_SIZE = 8
)(
  input  wire clock,
  input  wire rreq,
  output wire[BYTE_SIZE-1:0] rdata,
  input  wire wreq,
  input  wire[BYTE_SIZE-1:0] wdata,
  output wire empty,
  output wire full
);

  reg[BYTE_SIZE-1:0] fifo[(2**LOG_DEPTH)-1:0];
  reg[LOG_DEPTH-1:0] head = 0;
  reg[LOG_DEPTH-1:0] tail = 1;
  reg[LOG_DEPTH:0]   count = 0;

  wire[LOG_DEPTH-1:0] next_head = (head+1);
  wire[LOG_DEPTH-1:0] next_tail = (tail+1);

  assign rdata = fifo[head];
  assign empty = (count == 0);
  assign full = (count == 2**LOG_DEPTH);
  
  always @(posedge clock) begin
    if (rreq) begin
      head <= next_head;
    end
    if (wreq) begin 
      fifo[tail] <= wdata;
      tail <= next_tail;
    end

    if (rreq && !wreq) begin
      count <= count - 1;
    end else if (!rreq && wreq) begin
      count <= count + 1;
    end 
  end

endmodule

reg[3:0] COUNT = 0;
wire[3:0] rdata;
wire empty, full;

MyFifo#(2,3) fifo(
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
