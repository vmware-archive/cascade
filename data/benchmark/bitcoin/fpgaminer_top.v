//
//
// Copyright (c) 2011 fpgaminer@bitcoin-mining.com
//
//
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 
//

module fpgaminer_top (osc_clk);
  // The LOOP_LOG2 parameter determines how unrolled the SHA-256
  // calculations are. For example, a setting of 0 will completely
  // unroll the calculations, resulting in 128 rounds and a large, but
  // fast design.
  //
  // A setting of 1 will result in 64 rounds, with half the size and
  // half the speed. 2 will be 32 rounds, with 1/4th the size and speed.
  // And so on.
  //
  // Valid range: [0, 5]
  parameter LOOP_LOG2 = 0;
  // The DIFFICULT parameter determines how many trailing hash bits must
  // be zero before declaring success.
  // 
  // Valid range: [1, 256]
  parameter DIFFICULTY = 4;

  // No need to adjust these parameters
  localparam [5:0] LOOP = (6'd1 << LOOP_LOG2);
  // The nonce will always be larger at the time we discover a valid
  // hash. This is its offset from the nonce that gave rise to the valid
  // hash (except when LOOP_LOG2 == 0 or 1, where the offset is 131 or
  // 66 respectively).
  localparam [31:0] GOLDEN_NONCE_OFFSET = (32'd1 << (7 - LOOP_LOG2)) + 32'd1;

  input osc_clk;

  //// 
  reg [255:0] state = 0;
  reg [511:0] data = 0;
  reg [31:0] nonce = 32'h00000000;

  //// PLL
  wire hash_clk;
  assign hash_clk = osc_clk;

  //// Hashers
  wire [255:0] hash, hash2;
  reg [5:0] cnt = 6'd0;
  reg feedback = 1'b0;

  sha256_transform #(.LOOP(LOOP)) uut (
    .clk(hash_clk),
    .feedback(feedback),
    .cnt(cnt),
    .rx_state(state),
    .rx_input(data),
    .tx_hash(hash)
  );
  sha256_transform #(.LOOP(LOOP)) uut2 (
    .clk(hash_clk),
    .feedback(feedback),
    .cnt(cnt),
    .rx_state(256'h5be0cd191f83d9ab9b05688c510e527fa54ff53a3c6ef372bb67ae856a09e667),
    .rx_input({256'h0000010000000000000000000000000000000000000000000000000080000000, hash}),
    .tx_hash(hash2)
  );

  //// Control Unit
  reg [31:0] golden_nonce = 0;
  reg is_golden_ticket = 1'b0;
  reg feedback_d1 = 1'b1;
  wire [5:0] cnt_next;
  wire [31:0] nonce_next;
  wire feedback_next;
  
  assign cnt_next =  (LOOP == 1) ? 6'd0 : ((cnt + 6'd1) & (LOOP-1));
  // On the first count (cnt==0), load data from previous stage (no feedback)
  // on 1..LOOP-1, take feedback from current stage
  // This reduces the throughput by a factor of (LOOP), but also reduces the design size by the same amount
  assign feedback_next = (LOOP == 1) ? 1'b0 : (cnt_next != 0);
  assign nonce_next = feedback_next ? nonce : (nonce + 32'd1);

  always @ (posedge hash_clk)
  begin
    cnt <= cnt_next;
    feedback <= feedback_next;
    feedback_d1 <= feedback;

    // Give new data to the hasher
    state <= 255'd0;
    data <= {384'h000002800000000000000000000000000000000000000000000000000000000000000000000000000000000080000000, nonce_next, 96'd0};
    nonce <= nonce_next;

    // Check to see if the last hash generated is valid.
    // *** This last condition is here only because prior to this point, 
    // *** hash2 is undefined and we don't support x as a value
    is_golden_ticket <= (hash2[255:255-DIFFICULTY+1] == 0) && !feedback_d1 && (nonce > 32'h81);
    if(is_golden_ticket)
    begin
      // TODO: Find a more compact calculation for this
      if (LOOP == 1)
        golden_nonce <= nonce - 32'd131;
      else if (LOOP == 2)
        golden_nonce <= nonce - 32'd66;
      else
        golden_nonce <= nonce - GOLDEN_NONCE_OFFSET;
    end
  end
endmodule
