`include "share/test/benchmark/bitcoin/bitcoin.v"
Bitcoin#(.UNROLL(5), .DIFF(25)) bitcoin();
