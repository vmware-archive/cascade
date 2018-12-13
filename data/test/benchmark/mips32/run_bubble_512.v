include data/test/benchmark/mips32/mips32.v;

wire[31:0] instr, raddr;

(*__target="sw", __file="data/test/benchmark/mips32/run_bubble_512.hex"*)
Memory#(10,32) imem(
  .clock(clock.val),
  .raddr1(raddr),
  .rdata1(instr)
);

Mips32 mips32(
  .instr(instr),
  .raddr(raddr)
);
