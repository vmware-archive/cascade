include data/test/benchmark/bitcoin/sha-256-functions.v;

localparam ins1 = {
  {32'h 76543210},
  {32'h fedcba98},
  {32'h 02468ace},
  {32'h 13579bdf}
};

localparam ins2 = {
  {32'h ffffffff},
  {32'h cccccccc},
  {32'h 55555555},
  {32'h 11111111}
};

localparam ins3 = {
  {32'h eca86420},
  {32'h fdb97531},
  {32'h 01234567},
  {32'h 89abcdef}
};

genvar i;
for (i = 0; i < 4; i = i + 1) begin :b
  wire[31:0] in1 = (ins1 >> (32*i)) & 32'hffffffff;
  wire[31:0] in2 = (ins2 >> (32*i)) & 32'hffffffff;
  wire[31:0] in3 = (ins3 >> (32*i)) & 32'hffffffff;
  wire[31:0] out;
  ch ch_1(in1,in2,in3, out);
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
