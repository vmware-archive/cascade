module Control(instruction, reg_dst, jump, branch, mem_to_reg, alu_op, mem_write, alu_src, reg_write);
  input wire[31:0] instruction;
  output reg reg_dst;
  output reg jump;
  output reg branch;
  output reg mem_to_reg;
  output reg alu_op;
  output reg mem_write;
  output reg[1:0] alu_src;
  output reg reg_write;

  always @(*) begin
    case (instruction[31:26]) 
      // r-type
      6'd0:  begin
        // break 
        if (instruction[5:0] == 6'd13) begin
          reg_dst = 0; jump = 0; branch = 0; mem_to_reg = 0; alu_op = 0; mem_write = 0; alu_src = 2'b00; reg_write = 0;
        end 
        // sll, srl, sra
        else if (instruction[5:0] < 6'd4) begin
          reg_dst = 1; jump = 0; branch = 0; mem_to_reg = 0; alu_op = 1; mem_write = 0; alu_src = 2'b10; reg_write = 1;
        end
        // srlv, srav
        else if (instruction[5:0] < 6'd8) begin
          reg_dst = 1; jump = 0; branch = 0; mem_to_reg = 0; alu_op = 1; mem_write = 0; alu_src = 2'b11; reg_write = 1;
        end 
        // add, sub, and, or, xor, nor, slt
        else begin 
          reg_dst = 1; jump = 0; branch = 0; mem_to_reg = 0; alu_op = 1; mem_write = 0; alu_src = 2'b00; reg_write = 1;
        end
      end
      // j
      6'd2: begin
          reg_dst = 0; jump = 1; branch = 0; mem_to_reg = 0; alu_op = 0; mem_write = 0; alu_src = 2'b00; reg_write = 0;
      end
      // beq
      6'd4: begin
          reg_dst = 0; jump = 0; branch = 1; mem_to_reg = 0; alu_op = 0; mem_write = 0; alu_src = 2'b00; reg_write = 0;
      end
      // addi
      6'd8: begin
          reg_dst = 0; jump = 0; branch = 0; mem_to_reg = 0; alu_op = 0; mem_write = 0; alu_src = 2'b01; reg_write = 1;
      end
      // slti
      6'd10: begin
          reg_dst = 0; jump = 0; branch = 0; mem_to_reg = 0; alu_op = 0; mem_write = 0; alu_src = 2'b01; reg_write = 1;
      end
      // andi
      6'd12: begin
          reg_dst = 0; jump = 0; branch = 0; mem_to_reg = 0; alu_op = 0; mem_write = 0; alu_src = 2'b01; reg_write = 1;
      end
      // ori
      6'd13: begin
          reg_dst = 0; jump = 0; branch = 0; mem_to_reg = 0; alu_op = 0; mem_write = 0; alu_src = 2'b01; reg_write = 1;
      end
      // xori
      6'd14: begin
          reg_dst = 0; jump = 0; branch = 0; mem_to_reg = 0; alu_op = 0; mem_write = 0; alu_src = 2'b01; reg_write = 1;
      end
      // lui
      6'd15: begin
          reg_dst = 0; jump = 0; branch = 0; mem_to_reg = 0; alu_op = 0; mem_write = 0; alu_src = 2'b01; reg_write = 1;
      end
      // lw
      6'd35: begin
          reg_dst = 0; jump = 0; branch = 0; mem_to_reg = 1; alu_op = 0; mem_write = 0; alu_src = 2'b01; reg_write = 1;
      end 
      // sw
      6'd43: begin
          reg_dst = 0; jump = 0; branch = 0; mem_to_reg = 0; alu_op = 0; mem_write = 1; alu_src = 2'b01; reg_write = 0;
      end 

      // Careful! Don't create a latch here!
      default: begin
          reg_dst = 0; jump = 0; branch = 0; mem_to_reg = 0; alu_op = 0; mem_write = 0; alu_src = 2'b00; reg_write = 0;
      end 
    endcase
  end
endmodule
