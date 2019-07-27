module Memory#(
  parameter WORD_SIZE = 8,
  parameter ADDR_SIZE = 8,
  parameter PATH = "mem.dat"
)(
  input  wire clock,
  input  wire wen,
  input  wire[ADDR_SIZE-1:0] addr,
  input  wire[WORD_SIZE-1:0] wdata,
  output wire[WORD_SIZE-1:0] rdata
);
  // Derived Constants:
  localparam SIZE = 2**ADDR_SIZE;

  // Local Storage:
  reg[WORD_SIZE-1:0] data[SIZE-1:0];

  // Load initial values:
  integer fd = $fopen(PATH, "r+");
  integer i = 0;
  reg[WORD_SIZE-1:0] temp = 0;
  initial begin
    for (i = 0; i < SIZE-1; i=i+1) begin
      $fread(fd, temp);
      data[i] <= temp;
    end
  end
      
  // Latch writes on posedge of clock:
  always @(posedge clock) begin
    if (wen) begin
      data[addr] <= wdata;
    end
  end
  
  // Produce reads on every clock:
  assign rdata = data[addr];
endmodule
