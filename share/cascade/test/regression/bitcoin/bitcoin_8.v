`include "share/cascade/test/benchmark/bitcoin/sha-256-functions.v"
`include "share/cascade/test/benchmark/bitcoin/sha256_transform.v"

// NOTE: LOOP must be between 1 and 32 (and a power of 2!)
// NOTE: This module only sets tx_hash when feedback = 0
// NOTE: cnt must be masked against L-1

localparam cnts = {
  {6'b 000111},
  {6'b 000111},
  {6'b 000111},
  {6'b 000111}
};
localparam rx_states = {
  {8 {32'h 76543210}},
  {8 {32'h fedcba98}},
  {8 {32'h 02468ace}},
  {8 {32'h 13579bdf}}
};
localparam rx_inputs = {
  {16 {32'h 76543210}},
  {16 {32'h fedcba98}},
  {16 {32'h 02468ace}},
  {16 {32'h 13579bdf}}
};

genvar i;
for (i = 0; i < 4; i = i + 1) begin : b
  localparam L = 6'b1 << i;
  localparam f = 0;

  wire[5:0]   cnt      = (cnts      >>   (6*i)) & { 1 { 6'b111111}} & (L-1);
  wire[255:0] rx_state = (rx_states >> (256*i)) & { 8 {32'hffffffff}};
  wire[511:0] rx_input = (rx_inputs >> (512*i)) & {16 {32'hffffffff}};
  
  wire [255:0] tx_hash;
  sha256_transform#(.LOOP(L)) s(clock.val, f[0], cnt, rx_state, rx_input, tx_hash);
end

reg[9:0] COUNT = 0;
always @(posedge clock.val) begin
  COUNT <= COUNT + 1;
  if (COUNT == 127) begin
    $display("%h", b[0].tx_hash);
    $display("%h", b[1].tx_hash);
    $display("%h", b[2].tx_hash);
    $display("%h", b[3].tx_hash);
    $finish;
  end
end
