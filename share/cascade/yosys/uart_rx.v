`ifndef UART_RX
`define UART_RX

module UartRx#(
  parameter[31:0] DIVIDER = 25000000/115200
)(
	input wire clk,
	input wire serial,
	output reg[7:0] data = 0,
	output reg valid = 0
);

	reg[3:0] state = 0;
	reg[31:0] divcnt = 0;

	always @(posedge clk) begin
		divcnt <= divcnt + 1;
		valid <= 0;

		case (state)
			0: begin
        state <= !serial ? 1 : 0;
				divcnt <= 0;
			end
			1: begin
				if (2*divcnt > DIVIDER) begin
          state <= !serial ? 2 : 0;
	  		  divcnt <= 0;
				end
			end
			10: begin
				if (divcnt > DIVIDER) begin
					valid <= 1;
					state <= 0;
				end
			end
			default: begin
				if (divcnt > DIVIDER) begin
					data <= {serial, data[7:1]};
					state <= state + 1;
					divcnt <= 0;
				end
		  end
	  endcase
	end

endmodule

`endif
