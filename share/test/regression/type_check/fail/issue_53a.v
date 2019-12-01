module DUMMY(input wire[31:0] in);
endmodule

// Loop with identical name to variable

wire loop_0;

genvar i;
for(i = 0; i < 2; i=i+1) begin : loop_0
  wire [31:0] num;
  DUMMY dummy(num);
end

initial $finish(0);
