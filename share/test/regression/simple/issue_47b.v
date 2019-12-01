integer COUNT = 0;
reg[7:0] x = 0;

always @(posedge clock.val) begin
  COUNT <= COUNT + 1;
  case(COUNT)
    8'd0: $write(x); // Prints 0
    8'd1: x[1000] <= 1;  
    8'd2: $write(x); // Previous write should evaporate, prints 0
    8'd3: x[1000:1] <= -1;
    8'd4: $write(x); // Previous write should partially succeed, prints 254
    default: $finish;
  endcase
end
