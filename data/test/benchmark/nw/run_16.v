`include "data/test/benchmark/nw/constants_16.v"
`include "data/test/benchmark/nw/nw.v"

// Read input pairs one line at a time. This is an artifact from when we used
// to stream values with a cascade Fifo. This input structuring is no longer
// necessary.
localparam DATA_WIDTH = 2*LENGTH*CWIDTH;

// Instantiate compute grid:
reg [DATA_WIDTH-1:0] rdata;
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

reg done = 0;
reg once = 0;
reg[DATA_WIDTH-1:0] buffer = 0;
reg signed[SWIDTH-1:0] checksum = 0;

// While there are still inputs coming out of the fifo, sum the results

integer itr = 1;
integer s = $fopen("data/test/benchmark/nw/constants_8.hex", "r");
always @(posedge clock.val) begin
  $fread(s, buffer);
  if ($feof(s)) begin
    if (itr == 1024) begin
      done <= 1;
    end else begin
      itr <= itr + 1;
      $rewind(s);
      $fread(s, buffer);
    end
  end

  rdata <= buffer;
  once <= 1;

  // Exit case: print the checksum
  if (done) begin
    $write(checksum);
    $finish(0);
  end
  // Common case: Add score to the checksum
  else if (once) begin
    checksum <= checksum + score;
  end
end
