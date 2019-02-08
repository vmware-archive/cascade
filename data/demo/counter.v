// Connect counter variable to leds 
reg[7:0] counter = 0;
assign led.val = counter;

// Increment counter whenever pad is pressed 
reg[3:0] old_pad;
always @(posedge clock.val) begin
  old_pad <= pad.val;
  if ((old_pad[0] == 0) && (pad.val == 1)) begin
    counter <= counter + 1;
  end
end
