module Alu(ctrl, op1, op2, zero, result);
  input wire[3:0] ctrl;
  input wire[31:0] op1;
  input wire[31:0] op2;
  output wire zero;
  output reg[31:0] result; 

  always @(*) begin
    case (ctrl) 
      4'b0000: result = op1 << op2; // sll
      4'b0001: result = op1 >> op2; // srl
      4'b0010: result = op1 >>> op2; // sra
      4'b0011: result = op1 + op2; // add
      4'b0100: result = op1 - op2; // sub
      4'b0101: result = op1 & op2; // and
      4'b0110: result = op1 | op2; // or
      4'b0111: result = op1 ^ op2; // xor
      4'b1000: result = !(op1 | op2); // nor
      4'b1001: result = (op1 < op2) ? 1 : 0; // slt
      4'b1010: result = op2 << 16; // lui
      default: result = 0;
    endcase
  end
  assign zero = (result == 32'b0);
endmodule
