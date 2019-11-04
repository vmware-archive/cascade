`ifndef __SRC_TARGET_CORE_AVMM_AVALON_DEVICE_AVALON_WRAPPER_V
`define __SRC_TARGET_CORE_AVMM_AVALON_DEVICE_AVALON_WRAPPER_V

`include "src/target/core/avmm/avalon/device/program_logic.v"

reg read = 0;
reg write = 0;
reg[7:0] addr[1:0];
reg[7:0] data_in[3:0];
wire[31:0] data_out;
wire waitreq;
reg[7:0] temp = 0;

program_logic pl (
	.clk(clock.val),
	.reset(0),
	.s0_address({addr[1],addr[0]}),
	.s0_read(read),
	.s0_write(write),
	.s0_readdata(data_out),
	.s0_writedata({data_in[3],data_in[2],data_in[1],data_in[0]}),
	.s0_waitrequest(waitreq)
);

always @(posedge clock.val) begin
  /* TODO(eschkufz) DELETE THIS
  if (read) begin
    $display("READ: %h %h --- %d", {addr[1],addr[0]}, data_out, waitreq);
  end
  else if (write) begin
    $display("WRIT: %h %h --- %d", {addr[1],addr[0]}, {data_in[3],data_in[2],data_in[1],data_in[0]}, waitreq);
  end
  else begin
    $display("---");
  end
  */

	if ((read || write) && (!waitreq)) begin
		if (read) 
      $fwrite(ofd, "%c%c%c%c", data_out[31:24], data_out[23:16], data_out[15:8], data_out[7:0]);
		read <= 0;
		write <= 0;
	end
	if (!(read || write)) begin
		$fflush(ifd);
		$fscanf(ifd, "%c%c%c", temp, addr[1], addr[0]);
		if ($feof(ifd)) begin 
      // Does nothing
		end else begin
			if (temp[0]) 
        $fscanf(ifd, "%c%c%c%c", data_in[3], data_in[2], data_in[1], data_in[0]);
			read <= temp[1];
			write <= temp[0];
		end
	end
end

`endif
