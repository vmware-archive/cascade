// This program runs a counter on the leds and will either suspend or resume
// when button 0 or 1 is toggled to true. This program is compatible with any
// march file that exposes buttons and lights. Make sure to start this program
// with all buttons toggled to zero.

reg[7:0] counter = 0;
always @(posedge clock.val) begin
  counter <= counter + 1;
end
assign led.val = counter;

reg once = 0;
always @(posedge clock.val) begin
  if (!once && pad.val[0]) begin
    $save("data/demo/vmworld/backup.dat");
    once <= 1;
  end else if (!once && pad.val[1]) begin
    $restart("data/demo/vmworld/backup.dat");
    once <= 1;
  end
end
