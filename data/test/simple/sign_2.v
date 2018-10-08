wire signed [15:0] x = -1; 
wire[31:0] y = ~x;
wire[31:0] q = x + 1;

// These three prints statements all require sign-extension.
initial begin 
  // x is sign-extended to 32-bits due to y's bit-width in its initialization
  $write("%h", y); 
  // x is sign-extended to 32-bits due to its being added to a 32-bit constant.
  $write(q);
  // Same as above, but in a different context.
  $write(x+1);

  $finish;
end
