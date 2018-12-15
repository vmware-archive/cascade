reg pad = 0;
wire led;

module foo(x,y);
  input wire x;
  output wire y;
  wire temp;
  assign temp = x;
  assign y = temp;
endmodule

foo f(pad, led);

always @(posedge led) begin
  $write(led);
  $finish;
end

initial begin
  pad = 1;
end
