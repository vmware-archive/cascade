include data/test/benchmark/nw/constants_2.v;
include data/test/benchmark/nw/nw.v;

// Instantiate input fifo; we'll read input pairs one line at a time:
localparam DATA_WIDTH = 2*LENGTH*CWIDTH;
wire [DATA_WIDTH-1:0] rdata;
wire empty;
(*__target="sw", __file="data/test/benchmark/nw/constants_2.hex"*)
Fifo#(1, DATA_WIDTH) in (
  .clock(clock.val),
  .rreq(!empty),
  .rdata(rdata),
  .empty(empty)
);

// Instantiate compute grid:
wire [LENGTH*CWIDTH-1:0] s1 = rdata[2*LENGTH*CWIDTH-1:1*LENGTH*CWIDTH];
wire [LENGTH*CWIDTH-1:0] s2 = rdata[1*LENGTH*CWIDTH-1:0*LENGTH*CWIDTH];
wire signed[SWIDTH-1:0] score;
Grid#(
  .LENGTH(LENGTH),
  .CWIDTH(CWIDTH),
  .SWIDTH(SWIDTH),
  .MATCH(MATCH),
  .INDEL(INDEL),
  .MISMATCH(MISMATCH) 
) grid (
  .s1(s1),
  .s2(s2),
  .score(score)
);

// While there are still inputs coming out of the fifo, print the results:
reg signed[31:0] COUNT = -1;
reg signed[31:0] CHECKSUM = 0;
always @(posedge clock.val) begin
  // Base case: Ignore the first cycle where we get zeros out of the fifo
  if (COUNT == -1) begin
    COUNT <= COUNT + 1;
  end 
  // Exit case: Is the fifo finally empty?
  else if (empty) begin
    $write(CHECKSUM);
    $finish(0);
  end
  // Common case: increment count and add to the checksum
  else begin
    COUNT <= COUNT + 1;
    CHECKSUM <= CHECKSUM + score;
  end
end
