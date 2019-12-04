`include "share/cascade/test/benchmark/mips32/alu.v"
`include "share/cascade/test/benchmark/mips32/alu_control.v"
`include "share/cascade/test/benchmark/mips32/control.v"
`include "share/cascade/test/benchmark/mips32/mem.v"

module Mips32(
  input  wire[31:0] instr,
  output wire[31:0] raddr
);

  // Program Counter
  reg[31:0] pc = 0;
  wire[31:0] pc_4 = pc + 4;

  // Connection to Instruction Memory (64 aligned 32-bit words)
  // (Aligned word addressable, so addrs are shifted right 2)
  assign raddr = pc >> 2;

  // Control Unit
  wire reg_dst;
  wire jump;
  wire branch;
  wire mem_to_reg;
  wire alu_op;
  wire mem_write;
  wire[1:0] alu_src;
  wire c_reg_write;
  Control control(
    .instruction(instr), 
    .reg_dst(reg_dst),
    .jump(jump),
    .branch(branch),
    .mem_to_reg(mem_to_reg),
    .alu_op(alu_op),
    .mem_write(mem_write),
    .alu_src(alu_src),
    .reg_write(c_reg_write)
  );

  // Immediate Logic
  wire[4:0] shamt = instr[10:6];
  wire[31:0] imm = {instr[15] ? 16'b1 : 16'b0, instr[15:0]};

  // Jump Logic
  wire[31:0] jump_addr = {pc_4[31:28], instr[25:0], 2'b0};
  wire[31:0] branch_addr = pc_4 + (imm << 2);

  // Register File
  wire[31:0] reg_read1;
  wire[31:0] reg_read2;
  wire[31:0] reg_write;
  Mem#(5,32) regs(
    .clock(clock.val),
    .wen(c_reg_write),
    .raddr1(instr[25:21]),
    .rdata1(reg_read1),
    .raddr2(instr[20:16]),
    .rdata2(reg_read2),
    .waddr(reg_dst ? instr[15:11] : instr[20:16]),
    .wdata(reg_write)
  );

  // ALU Control
  wire[3:0] ctrl;
  AluControl alu_control(
    .alu_op(alu_op),
    .op(instr[31:26]),
    .ffield(instr[5:0]),
    .ctrl(ctrl) 
  );

  // ALU
  wire zero;
  wire[31:0] result;
  Alu alu(
    .ctrl(ctrl),
    .op1(alu_src[1] ? reg_read2 : reg_read1),
    .op2(alu_src[1] ? (alu_src[0] ? reg_read1 : shamt) : (alu_src[0] ? imm : reg_read2)),
    .zero(zero),
    .result(result)
  );

  // Data Memory (2^16 aligned 32-bit words)
  // (Aligned word addressable, so addres are shifted right 2)
  wire[31:0] mem_read;
  Mem#(7,32) dmem(
    .clock(clock.val),
    .wen(mem_write),
    .raddr1(result >> 2),
    .rdata1(mem_read),
    .waddr(result >> 2),
    .wdata(reg_read2)
  );
  assign reg_write = mem_to_reg ? mem_read : result;

  // Main Loop
  always @(posedge clock.val) begin
    if (instr[5:0] == 6'd13) begin
      $write(reg_read1);
      $finish;
    end

    if (jump) begin
      pc <= jump_addr;
    end else if (branch & zero) begin
      pc <= branch_addr;
    end else begin
      pc <= pc_4;
    end
  end

endmodule
