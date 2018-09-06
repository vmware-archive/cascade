reg pad = 0;
wire temp = pad;
wire led = temp;

always @(posedge led) begin
  $write(led);
  $finish;
end

initial begin
  pad = 1;
end
