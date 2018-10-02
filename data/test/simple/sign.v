integer a = -4'd12;
reg[15:0] ra;
reg[15:0] rb;
integer b;
integer c = -4'd12 / 3;
reg[15:0] d = -12 / 3;
reg signed[15:0] e = -12 / 3;
reg signed[15:0] f = -4'sd12 / 3;

initial begin
  // Section 5.1.3 
  // Using integer numbers in expressions

  $write(-12/3);     // -4
  $write(-'d12/3);   // 1431655761
  $write(-'sd12/3);  // -4
  $write(-4'sd12/3); // 1

  // Section 5.1.6
  // Arithmetic expressions with regs and integers
  ra = a / 3; 
  rb = -4'd12;
  b = rb / 3;  
  $write(ra);        // 65532
  $write(b);         // 21841
  $write(c);         // 1431655761
  $write(d);         // 65532
  $write(e);         // -4
  $write(f);         // 1

  $finish;
end
