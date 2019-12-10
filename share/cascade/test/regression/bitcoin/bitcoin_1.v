`include "share/cascade/test/benchmark/bitcoin/sha-256-functions.v"

localparam ins = {
  {32'h 76543210},
  {32'h fedcba98},
  {32'h 02468ace},
  {32'h 13579bdf}
};

genvar i;
for (i = 0; i < 4; i = i + 1) begin :b
  wire[31:0] in = (ins >> (32*i)) & 32'hffffffff;
  wire[31:0] out;
  e0 e0_1(in, out);
end

always @(posedge clock.val) begin
  if (b[0].out && b[1].out && b[2].out && b[3].out) begin
    $display("%h", b[0].out);
    $display("%h", b[1].out);
    $display("%h", b[2].out);
    $display("%h", b[3].out);
    $finish;
  end
end
