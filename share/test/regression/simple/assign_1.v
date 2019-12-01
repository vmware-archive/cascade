reg pad = 0;
wire led = pad;

always @(posedge led) begin
  $write(led);
  $finish;
end

initial begin
  pad = 1;
end
