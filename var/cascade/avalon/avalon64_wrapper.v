`ifndef __VAR_CASCADE_AVALON_AVALON64_WRAPPER_V
`define __VAR_CASCADE_AVALON_AVALON64_WRAPPER_V

`include "var/cascade/avalon/program_logic.v"

reg read = 0;
reg write = 0;
reg[7:0] addr[3:0];
reg[7:0] data_in[7:0];
wire[63:0] data_out;
wire waitreq;
reg[7:0] temp = 0;

program_logic pl (
	.clk(clock.val),
	.reset(0),
	.s0_address({addr[3],addr[2],addr[1],addr[0]}),
	.s0_read(read),
	.s0_write(write),
	.s0_readdata(data_out),
	.s0_writedata({data_in[7],data_in[6],data_in[5],data_in[4],data_in[3],data_in[2],data_in[1],data_in[0]}),
	.s0_waitrequest(waitreq)
);

always @(posedge clock.val) begin
	if ((read || write) && (!waitreq)) begin
		if (read) 
      $fwrite(ofd, "%c%c%c%c%c%c%c%c", data_out[7:0], data_out[15:8], data_out[23:16], data_out[31:24], data_out[39:32], data_out[47:40], data_out[55:48], data_out[63:56]);
		read <= 0;
		write <= 0;
	end
	if (!(read || write)) begin
		$fflush(ifd);
		$fscanf(ifd, "%c%c%c%c%c", temp, addr[0], addr[1], addr[2], addr[3]);
		if ($feof(ifd)) begin 
      // Does nothing
		end else begin
			if (temp[0]) 
        $fscanf(ifd, "%c%c%c%c%c%c%c%c", data_in[0], data_in[1], data_in[2], data_in[3], data_in[4], data_in[5], data_in[6], data_in[7]);
			read <= temp[1];
			write <= temp[0];
		end
	end
end

`endif
