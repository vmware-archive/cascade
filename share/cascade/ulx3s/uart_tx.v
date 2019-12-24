`ifndef UART_TX
`define UART_TX

module UartTx#(
  parameter[31:0] DIVIDER = 25000000/115200
)(
	input wire clk,
	output wire serial,
	input wire enable,
	input wire[7:0] data,
	output wire ready
);

	reg[9:0] packet = 10'h3ff;
	reg[3:0] bitcnt = 0;
	reg[31:0] divcnt = 0;

	always @(posedge clk) begin
		divcnt <= divcnt + 1;

		if (enable && !bitcnt) begin
			packet <= {1'b1, data, 1'b0};
			bitcnt <= 10;
			divcnt <= 0;
		end else if ((divcnt > DIVIDER) && bitcnt) begin
			packet <= {1'b1, packet[9:1]};
			bitcnt <= bitcnt - 1;
			divcnt <= 0;
		end
	end

	assign serial = packet[0];
	assign ready = !bitcnt && !enable; 

endmodule

`endif
