integer val;
always @(posedge clock.val) begin
  $write("Enter value: ");
  $fscanf(STDIN, "%d", val);
  $display("Echo: %d", val);
end
