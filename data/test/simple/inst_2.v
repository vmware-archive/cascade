reg[1:0] pad = 0;
reg[1:0] led = 0;

module foo(x,y);
  input wire[1:0] x;
  output wire[1:0] y;
  assign y = x;
endmodule

foo f(2*pad, led);

always @(posedge led) begin
  $write(led);
  $finish;
end

initial begin
  pad = 1;
end
