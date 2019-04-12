include data/test/benchmark/mips32/mips32.v;

wire[31:0] raddr;
wire[31:0] instr;
Mem#(6,32) imem(
  .clock(clock.val),
  .raddr1(raddr),
  .rdata1(instr)
);

stream s = $fopen("data/test/benchmark/mips32/run_bubble_128_1024.hex");
integer i = 0;
reg[31:0] val = 0;
initial begin
  for (i = 0; i < 63; i = i + 1) begin
    $get(s, val);
    imem.mem[i] = val;
  end
end 

Mips32 mips32(
  .instr(instr),
  .raddr(raddr)
);
