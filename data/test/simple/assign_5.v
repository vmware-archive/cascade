reg pad = 0;
wire led = 0;

module foo(x,y);
  input wire x;
  output wire y;
  assign y = x;
endmodule

module bar(x,y);
  input wire x;
  output wire y;
  wire temp;
  foo f1(x, temp);
  foo f2(temp, y);
endmodule

bar b(pad, led);

always @(posedge led) begin
  $write(led);
  $finish;
end

initial begin
  pad = 1;
end
