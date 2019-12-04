integer COUNT = 0;
reg x[1:0];

always @(posedge clock.val) begin
  COUNT <= COUNT + 1;
  case(COUNT)
    0: x[0] <= 0;
    1: $write(x[0]); // Previous write suceeds, prints 0
    2: x[1000] <= 1;
    3: $write(x[0]); // Previous write evaporates, prints 0
    4: $write(x[1]); // Previous write evaporates, prints 0
    5: $finish;
  endcase
end

