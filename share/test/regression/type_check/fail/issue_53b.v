module DUMMY(input wire[31:0] in);
endmodule

// Identically named loops

genvar i;
for(i = 0; i < 2; i=i+1) begin : loop_0
  wire [31:0] num;
  DUMMY dummy(num);
end

genvar j;
for(j = 0; j < 2; j=j+1) begin : loop_0
  wire [31:0] num2;
  DUMMY dummy(num2);
end

initial $finish(0);
