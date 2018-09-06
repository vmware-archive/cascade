genvar i;
always @(posedge clock.val) begin
  case (i) default: $display(1); endcase
end
