include data/test/bitcoin/sha-256-functions.v;
include data/test/bitcoin/sha256_transform.v;

localparam ks = {
  {32'h 76543210},
  {32'h fedcba98},
  {32'h 02468ace},
  {32'h 13579bdf}
};
localparam rx_ws = {
  {16 {32'h 76543210}},
  {16 {32'h fedcba98}},
  {16 {32'h 02468ace}},
  {16 {32'h 13579bdf}}
};
localparam rx_states = {
  {8 {32'h 76543210}},
  {8 {32'h fedcba98}},
  {8 {32'h 02468ace}},
  {8 {32'h 13579bdf}}
};

genvar i;
for (i = 0; i < 4; i = i + 1) begin : b
  wire[31:0]  k        = (ks        >>  (32*i)) & { 1 {32'hffffffff}};
  wire[511:0] rx_w     = (rx_ws     >> (512*i)) & {16 {32'hffffffff}};
  wire[255:0] rx_state = (rx_states >> (256*i)) & { 8 {32'hffffffff}};

  wire [511:0] tx_w;
  wire [255:0] tx_state;
  sha256_digester s(clock.val, k, rx_w, rx_state, tx_w, tx_state);
end

always @(posedge clock.val) begin
  if (b[0].tx_w && b[1].tx_w && b[2].tx_w && b[3].tx_w) begin
    $display("%h %h", b[0].tx_w, b[0].tx_state); 
    $display("%h %h", b[1].tx_w, b[1].tx_state); 
    $display("%h %h", b[2].tx_w, b[2].tx_state); 
    $display("%h %h", b[3].tx_w, b[3].tx_state); 
    $finish;
  end
end
