`include "share/cascade/test/benchmark/mips32/mips32.v"

reg[31:0] imem[63:0];

integer s = $fopen("share/cascade/test/benchmark/mips32/sum.hex", "r");
integer i = 0;
reg[31:0] val = 0;
initial begin
  for (i = 0; i < 63; i = i + 1) begin
    $fread(s, val);
    imem[i] <= val;
  end
end 

wire[31:0] addr;
wire[31:0] instr = imem[addr];
Mips32 mips32(
  .instr(instr),
  .raddr(addr)
);
