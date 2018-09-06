include sha-256-functions.v;
include sha256_transform.v;
include fpgaminer_top.v;

fpgaminer_top#(.LOOP_LOG2(0),.DIFFICULTY(12)) miner(clock.val);

always @(posedge clock.val) begin
  if (miner.golden_nonce) begin
    $display("%h %h", miner.golden_nonce, miner.nonce);
    $finish;
  end
end
