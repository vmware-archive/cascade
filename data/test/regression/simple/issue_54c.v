// A two-element circular assignment
wire x,y;
assign y = x;
assign x = y;

// A three-element circular assignment
wire a,b,c;
assign a = b;
assign b = c;
assign c = a;

// An assignment chain that ends in a circular assignment
wire r,s,t;
assign r = s;
assign s = t;
assign t = s;

// The values we observe aren't particularly important (this is an undefined
// program), though we know that cascade arbitrarily sets floating wires to 0.
// What matters is that the code doesn't enter into an infinite loop or crash.
initial begin
  $write("%d%d", x, y);
  $write("%d%d%d", a, b, c);
  $write("%d%d%d", r, s, t);
  $finish;
end
