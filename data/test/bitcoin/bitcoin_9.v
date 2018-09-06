include data/test/bitcoin/sha-256-functions.v;
include data/test/bitcoin/sha256_transform.v;
include data/test/bitcoin/fpgaminer_top.v;

fpgaminer_top#(.LOOP_LOG2(0)) miner(clock.val);

always @(posedge clock.val) begin
  if (miner.golden_nonce) begin 
    $write("%h %h", miner.golden_nonce, miner.nonce);
    $finish;
  end
end
