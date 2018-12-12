include data/benchmark/bitcoin/sha-256-functions.v;
include data/benchmark/bitcoin/sha256_transform.v;
include data/benchmark/bitcoin/fpgaminer_top.v;

module Bitcoin();

  parameter DIFF = 2;
  parameter FINISH = 1;

  fpgaminer_top#(.LOOP_LOG2(0),.DIFFICULTY(DIFF)) miner(clock.val);

  always @(posedge clock.val) begin
    if (miner.golden_nonce) begin
      $display("%h %h", miner.golden_nonce, miner.nonce);
      $finish(FINISH);
    end
  end

endmodule
