// This reference implementation should compile with iverilog. It includes a
// proxy for the globally visible cascade Clock.

`include "sha-256-functions.v"
`include "sha256_transform.v"
`include "fpgaminer_top.v"

module Clock();
  reg val = 0;
  always @(val) begin #1; val <= ~val; end
endmodule

module main;
  Clock clock();
  fpgaminer_top#(.LOOP_LOG2(0), .DIFFICULTY(12)) miner(clock.val);

  always @(posedge clock.val) begin
    if (miner.golden_nonce) begin
      $display("%h %h", miner.golden_nonce, miner.nonce);
      $finish;
    end
  end
endmodule
