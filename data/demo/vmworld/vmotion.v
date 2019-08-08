// This program runs a counter on its leds and whenever a button is toggled to
// the on positions, vmotions to that fpga. For instance, if you toggle button
// 3, the counter will jump to the third fpga. Make sure to push the buttons back
// down after you use them.

// This program should be started with one of the four fpga_{2,3,4,5} march files,
// and all of the buttons in the down position (towards the button of the board)

reg[7:0] counter = 0;
always @(posedge clock.val) begin
  counter <= counter + 1;
end
assign led.val = counter;

always @(posedge clock.val) begin
  if (pad.val[0]) begin
    $display("vMotion -> fpga 2");      
    $retarget("vmworld/fpga_2");
  end else if (pad.val[1]) begin
    $display("vMotion -> fpga 3");      
    $retarget("vmworld/fpga_3");
  end else if (pad.val[2]) begin
    $display("vMotion -> fpga 4");      
    $retarget("vmworld/fpga_4");
  end else if (pad.val[3]) begin
    $display("vMotion -> fpga 5");      
    $retarget("vmworld/fpga_5");
  end
end
