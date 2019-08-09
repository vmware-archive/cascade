// Prints the value of the buttons to the terminal whenever they are changed.
// This program is compatible with any march file that exposes buttons.

always @(pad.val) begin
  $display("Buttons changed value: %b (%d)", pad.val, pad.val);
end
