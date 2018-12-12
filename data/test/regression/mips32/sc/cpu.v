include data/test/regression/mips32/sc/alu.v;
include data/test/regression/mips32/sc/alu_control.v;
include data/test/regression/mips32/sc/control.v;
include data/test/regression/mips32/sc/mem.v;

// Program Counter

reg[31:0] pc = 0;
wire[31:0] pc_4 = pc + 4;

// Instruction Memory (64 aligned 32-bit words)
// (Aligned word addressable, so addrs are shifted right 2)

wire[31:0] instruction;
(*__file="data/test/regression/mips32/sc/imem.mem"*) 
Memory#(6,32) imem(
  .clock(clock.val),
  .raddr1(pc >> 2), 
  .rdata1(instruction)
);

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
  .instruction(instruction), 
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
wire[4:0] shamt = instruction[10:6];
wire[31:0] imm = {instruction[15] ? 16'b1 : 16'b0, instruction[15:0]};

// Jump Logic

wire[31:0] jump_addr = {pc_4[31:28], instruction[25:0], 2'b0};
wire[31:0] branch_addr = pc_4 + (imm << 2);

// Register File

wire[31:0] reg_read1;
wire[31:0] reg_read2;
wire[31:0] reg_write;
Mem#(5,32) regs(
  .clock(clock.val),
  .wen(c_reg_write),
  .raddr1(instruction[25:21]),
  .rdata1(reg_read1),
  .raddr2(instruction[20:16]),
  .rdata2(reg_read2),
  .waddr(reg_dst ? instruction[15:11] : instruction[20:16]),
  .wdata(reg_write)
);

// ALU Control

wire[3:0] ctrl;
AluControl alu_control(
  .alu_op(alu_op),
  .op(instruction[31:26]),
  .ffield(instruction[5:0]),
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

// Data Memory (32 aligned 32-bit words)
// (Aligned word addressable, so addres are shifted right 2)

wire[31:0] mem_read;
Mem#(10,32) dmem(
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
  if (instruction[5:0] == 6'd13) begin
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
