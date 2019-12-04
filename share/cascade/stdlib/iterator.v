module Iterator#(
  parameter WORD_SIZE = 8,
  parameter PATH = "mem.dat"
)(
  input  wire clock,
  input  wire reset,
  input  wire read,
  output reg[WORD_SIZE-1:0] val,
  output reg eof
);

  // Open file for reading
  integer fd = $fopen(PATH, "r");
  initial begin
    eof = $feof(fd);
  end

  // Reset or read next value based on inputs:
  always @(posedge clock.val) begin
    if (reset) begin
      $rewind(fd);
      eof = $feof(fd);
    end else if (read) begin
      $fread(fd, val);
      eof = $feof(fd);
    end
  end

endmodule
