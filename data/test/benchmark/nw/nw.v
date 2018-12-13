// Grid Elements:
module Cell#(
  // Number of characters per string
  parameter LENGTH = 10,
  // Number of bits per character
  parameter CWIDTH = 2,
  // Number of bits per score
  parameter SWIDTH = 16,
  // Weights
  parameter signed MATCH = 1,
  parameter signed INDEL = -1,
  parameter signed MISMATCH = -1
)(
  // Incoming top, left, and top-Left scores
  input wire signed[SWIDTH-1:0] t,
  input wire signed[SWIDTH-1:0] l,
  input wire signed[SWIDTH-1:0] tl,
  // Inputs for this cell
  input wire[CWIDTH-1:0] in1,
  input wire[CWIDTH-1:0] in2,
  // Score for this cell
  output wire signed[SWIDTH-1:0] out
);
  // Calculate scores
  wire signed [SWIDTH-1:0] t_score  = t + INDEL;
  wire signed [SWIDTH-1:0] l_score  = l + INDEL;
  wire signed [SWIDTH-1:0] tl_score = tl + (in1 == in2 ? MATCH : MISMATCH);

  // Return max
  wire signed [SWIDTH-1:0] temp = (t_score >= l_score ? t_score : l_score);
  assign out = (temp >= tl_score ? temp : tl_score);
endmodule

// Grid:
module Grid#(
  // Number of characters per string
  parameter LENGTH = 10,
  // Number of bits per character
  parameter CWIDTH = 2,
  // Number of bits per score
  parameter SWIDTH = 16,
  // Weights
  parameter signed MATCH = 1,
  parameter signed INDEL = -1,
  parameter signed MISMATCH = -1
)(
  // Input strings
  input wire signed[LENGTH*CWIDTH-1:0] s1,
  input wire signed[LENGTH*CWIDTH-1:0] s2,
  // Match score
  output wire signed[SWIDTH-1:0] score
);
  genvar i;
  genvar j;
  for (i = 1; i <= LENGTH; i = i+1) begin : ROW
    for (j = 1; j <= LENGTH; j = j+1) begin : COL
      localparam s1_idx = CWIDTH*(LENGTH-i);
      localparam s2_idx = CWIDTH*(LENGTH-j);

      wire[CWIDTH-1:0] c1 = s1[s1_idx+:CWIDTH];
      wire[CWIDTH-1:0] c2 = s2[s2_idx+:CWIDTH];
      wire signed [SWIDTH-1:0] out;

      if ((i == 1) && (j == 1)) begin
        Cell#(LENGTH,CWIDTH,SWIDTH,MATCH,INDEL,MISMATCH) c(-1, -1, 0, c1, c2, out);
      end else if (i == 1) begin 
        Cell#(LENGTH,CWIDTH,SWIDTH,MATCH,INDEL,MISMATCH) c(-j, ROW[1].COL[j-1].out, -j+1, c1, c2, out);
      end else if (j == 1) begin 
        Cell#(LENGTH,CWIDTH,SWIDTH,MATCH,INDEL,MISMATCH) c(ROW[i-1].COL[j].out, -i, -i+1, c1, c2, out);
      end else begin
        Cell#(LENGTH,CWIDTH,SWIDTH,MATCH,INDEL,MISMATCH) c(ROW[i-1].COL[j].out, ROW[i].COL[j-1].out, ROW[i-1].COL[j-1].out, c1, c2, out);
      end
    end
  end
  assign score = ROW[LENGTH].COL[LENGTH].out;
endmodule
