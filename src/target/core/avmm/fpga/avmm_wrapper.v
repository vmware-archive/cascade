`ifndef __AVALON_WRAPPER_V
`define __AVALON_WRAPPER_V

`include "src/target/core/avmm/fpga/program_logic.v"

reg read = 0;
reg write = 0;
reg[15:0] addr = 0;
reg[31:0] data_in = 0;
wire[31:0] data_out;
wire waitreq;
reg[7:0] temp = 0;

program_logic pl (
	.clk(clock.val),
	.reset(0),
	.s0_address(addr),
	.s0_read(read),
	.s0_write(write),
	.s0_readdata(data_out),
	.s0_writedata(data_in),
	.s0_waitrequest(waitreq)
);

always @(posedge clock.val) begin
	if ((read || write) && (!waitreq)) begin
		if (read) 
      $fwrite(ofd, "%c%c%c%c", data_out[31:24], data_out[23:16], data_out[15:8], data_out[7:0]);
		read <= 0;
		write <= 0;
	end
	if (!(read || write)) begin
		$fflush(ifd);
		$fscanf(ifd, "%c%c%c", temp[7:0], addr[15:8], addr[7:0]);
		if ($feof(ifd)) begin 
      // Does nothing
		end else begin
			if (temp[0]) 
        $fscanf(ifd, "%c%c%c%c", data_in[31:24], data_in[23:16], data_in[15:8], data_in[7:0]);
			read <= temp[1];
			write <= temp[0];
		end
	end
end

`endif
