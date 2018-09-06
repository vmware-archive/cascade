module AluControl(alu_op, op, ffield, ctrl);
  input wire alu_op;
  input wire[5:0] op;
  input wire[5:0] ffield;
  output reg[3:0] ctrl;

  always @(*) begin
    // Switch on opcode
    if (alu_op == 0) begin
      case (op) 
        6'd4:  ctrl = 4'b0100; // beq:  sub
        6'd8:  ctrl = 4'b0011; // addi: add
        6'd10: ctrl = 4'b1001; // slti: slt
        6'd12: ctrl = 4'b0101; // andi: and
        6'd13: ctrl = 4'b0110; // ori:  or
        6'd14: ctrl = 4'b0111; // xori: xor
        6'd15: ctrl = 4'b1010; // lui:  lui
        6'd35: ctrl = 4'b0011; // lw:   add
        6'd43: ctrl = 4'b0011; // sw:   add
        default: ctrl = 0;
      endcase
    end 
    // Switch on function field  
    else begin
      case (ffield) 
        6'd0:  ctrl = 4'b0000; // sll:  sll 
        6'd2:  ctrl = 4'b0001; // srl:  srl
        6'd3:  ctrl = 4'b0010; // sra:  sra
        6'd6:  ctrl = 4'b0001; // srlv: srl
        6'd7:  ctrl = 4'b0010; // srav: sra
        6'd32: ctrl = 4'b0011; // add:  add
        6'd34: ctrl = 4'b0100; // sub:  sub
        6'd36: ctrl = 4'b0101; // and:  and
        6'd37: ctrl = 4'b0110; // or:   or
        6'd38: ctrl = 4'b0111; // xor:  xor
        6'd39: ctrl = 4'b1000; // nor:  nor
        6'd42: ctrl = 4'b1001; // slt:  slt
        default: ctrl = 0;
      endcase
    end
  end
endmodule
