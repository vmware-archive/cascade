module M0(__clk,__read,__vid,__in,__out,__wait);
	input wire __clk;
	input wire __read;
	input wire[13:0] __vid;
	input wire[31:0] __in;
	output reg[31:0] __out;
	output wire __wait;
	reg __read_prev = 0;
	wire __read_request;
	reg[31:0] __var[16:0];
	reg[31:0] __expr[15:0];
	reg[31:0] __l10_next;
	reg[31:0] __l11_next;
	reg signed[31:0] __l12_next;
	reg signed[31:0] __l13_next;
	reg[31:0] __l14_next;
	reg[7:0] __l8_next;
	reg[31:0] __l9_next;
	reg[31:0] __prev_update_mask = 0;
	reg[31:0] __update_mask = 0;
	wire[31:0] __l10;
	assign __l10 = {__var[3]};
	wire[31:0] __l11;
	assign __l11 = {__var[4]};
	wire signed[31:0] __l12;
	assign __l12 = {__var[5]};
	wire signed[31:0] __l13;
	assign __l13 = {__var[6]};
	wire[31:0] __l14;
	assign __l14 = {__var[7]};
	wire[7:0] __l8;
	assign __l8 = {__var[1]};
	wire[31:0] __l9;
	assign __l9 = {__var[2]};
	wire __x1;
	assign __x1 = {__var[0]};
	wire[31:0] __update_queue;
	wire __there_are_updates;
	wire __apply_updates;
	wire __drop_updates;
	wire __there_were_tasks;
	wire __all_final;
	wire __continue;
	wire __reset;
	wire __done;
	reg __x1_prev;
	wire __x1_posedge;
	wire __any_triggers;
	reg[31:0] __open_loop = 0;
	wire __open_loop_tick;
	reg[31:0] __task_id[0:0];
	reg[31:0] __state[0:0];
	always @(posedge __clk) __read_prev <= __read;
	assign __read_request = (!__read_prev && __read);
	assign __update_queue = (__prev_update_mask ^ __update_mask);
	assign __there_are_updates = |__update_queue;
	assign __apply_updates = ((__read_request && (__vid == 9)) || (__there_are_updates && __open_loop_tick));
	assign __drop_updates = (__read_request && (__vid == 10));
	always @(posedge __clk) __prev_update_mask <= ((__apply_updates || __drop_updates) ? __update_mask : __prev_update_mask);
	assign __there_were_tasks = |{(__task_id[0] != 0)};
	assign __all_final = &{(__state[0] == 33)};
	assign __continue = ((__read_request && (__vid == 12)) || (!__all_final && !__there_were_tasks));
	assign __reset = (__read_request && (__vid == 13));
	assign __done = (__all_final && !__there_were_tasks);
	always @(posedge __clk) begin
		__x1_prev <= __x1;
	end 
	assign __x1_posedge = ((__x1_prev == 0) && (__x1 == 1));
	assign __any_triggers = |{__x1_posedge};
	always @(posedge __clk) __open_loop <= ((__read_request && (__vid == 15)) ? __in : (__open_loop_tick ? (__open_loop - 1) : __open_loop));
	assign __open_loop_tick = (__all_final && (!__any_triggers && (__open_loop > 0)));
	always @(posedge __clk) begin
		if (__continue) 
			case (__state[0])
				0: begin
					__task_id[0] <= 1;
					__state[0] <= 1;
				end 
				1: begin
					__task_id[0] <= 2;
					__state[0] <= 2;
				end 
				2: begin
					__task_id[0] <= 3;
					__state[0] <= 3;
				end 
				3: begin
					__task_id[0] <= 0;
					if (__expr[0]) 
						__state[0] <= 4;
					else 
						__state[0] <= 10;
				end 
				4: begin
					__task_id[0] <= 0;
					if ((__l12 == 1)) 
						__state[0] <= 5;
					else 
						__state[0] <= 8;
				end 
				5: begin
					__task_id[0] <= 4;
					__state[0] <= 6;
				end 
				6: begin
					__task_id[0] <= 5;
					__state[0] <= 7;
				end 
				7: begin
					__task_id[0] <= 6;
					__state[0] <= 9;
				end 
				8: begin
					__l12_next <= (__l12 + 1);
					__update_mask[5+:1] <= ~__prev_update_mask[5+:1];
					__task_id[0] <= 7;
					__state[0] <= 9;
				end 
				9: begin
					__task_id[0] <= 0;
					__state[0] <= 33;
				end 
				10: begin
					if ((__l14 > 0)) begin
						__l11_next <= (__l11 + 1);
						__update_mask[4+:1] <= ~__prev_update_mask[4+:1];
					end 
					__task_id[0] <= 0;
					case (__l14)
						32'd0: __state[0] <= 11;
						32'd1: __state[0] <= 12;
						32'd2: __state[0] <= 13;
						32'd3: __state[0] <= 14;
						32'd4: __state[0] <= 15;
						32'd5: __state[0] <= 16;
						32'd6: __state[0] <= 17;
						32'd7: __state[0] <= 18;
						32'd8: __state[0] <= 19;
						32'd9: __state[0] <= 20;
						32'd10: __state[0] <= 21;
						32'd11: __state[0] <= 22;
						32'd12: __state[0] <= 23;
						32'd13: __state[0] <= 24;
						32'd14: __state[0] <= 25;
						32'd15: __state[0] <= 26;
						32'd16: __state[0] <= 27;
						default: __state[0] <= 28;
					endcase
				end 
				11: begin
					__l14_next <= 1;
					__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				12: begin
					case (__l8)
						8'h41: begin
							__l14_next <= 2;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						8'h54: begin
							__l14_next <= 3;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				13: begin
					case (__l8)
						8'h63: begin
							__l14_next <= 10;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				14: begin
					case (__l8)
						8'h48: begin
							__l14_next <= 4;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				15: begin
					case (__l8)
						8'h45: begin
							__l14_next <= 5;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				16: begin
					case (__l8)
						8'h20: begin
							__l14_next <= 6;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				17: begin
					case (__l8)
						8'h45: begin
							__l14_next <= 7;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				18: begin
					case (__l8)
						8'h4e: begin
							__l14_next <= 8;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				19: begin
					case (__l8)
						8'h44: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l9_next <= (__l9 + 1);
							__update_mask[2+:1] <= ~__prev_update_mask[2+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				20: begin
					case (__l8)
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				21: begin
					case (__l8)
						8'h68: begin
							__l14_next <= 11;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				22: begin
					case (__l8)
						8'h69: begin
							__l14_next <= 12;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				23: begin
					case (__l8)
						8'h6c: begin
							__l14_next <= 13;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				24: begin
					case (__l8)
						8'h6c: begin
							__l14_next <= 14;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				25: begin
					case (__l8)
						8'h65: begin
							__l14_next <= 15;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				26: begin
					case (__l8)
						8'h73: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l9_next <= (__l9 + 1);
							__update_mask[2+:1] <= ~__prev_update_mask[2+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				27: begin
					case (__l8)
						default: begin
							__l10_next <= (__l11 + 1);
							__update_mask[3+:1] <= ~__prev_update_mask[3+:1];
							__l14_next <= 1;
							__update_mask[7+:1] <= ~__prev_update_mask[7+:1];
						end 
					endcase
					__task_id[0] <= 0;
					__state[0] <= 32;
				end 
				28: begin
					__task_id[0] <= 8;
					__state[0] <= 29;
				end 
				29: begin
					__task_id[0] <= 9;
					__state[0] <= 30;
				end 
				30: begin
					__task_id[0] <= 10;
					__state[0] <= 31;
				end 
				31: begin
					__task_id[0] <= 11;
					__state[0] <= 32;
				end 
				32: begin
					__task_id[0] <= 0;
					__state[0] <= 33;
				end 
				default: begin
					__task_id[0] <= 0;
					__state[0] <= 33;
				end 
			endcase
		else begin
			__state[0] <= (__reset ? 33 : (__x1_posedge ? 0 : __state[0]));
		end 
		__var[0] <= (__open_loop_tick ? {31'd0,~__x1} : ((__read_request && (__vid == 0)) ? __in : __var[0]));
		__var[1] <= ((__read_request && (__vid == 1)) ? __in : ((__apply_updates && __update_queue[1]) ? __l8_next[7:0] : __var[1]));
		__var[2] <= ((__read_request && (__vid == 2)) ? __in : ((__apply_updates && __update_queue[2]) ? __l9_next[31:0] : __var[2]));
		__var[3] <= ((__read_request && (__vid == 3)) ? __in : ((__apply_updates && __update_queue[3]) ? __l10_next[31:0] : __var[3]));
		__var[4] <= ((__read_request && (__vid == 4)) ? __in : ((__apply_updates && __update_queue[4]) ? __l11_next[31:0] : __var[4]));
		__var[5] <= ((__read_request && (__vid == 5)) ? __in : ((__apply_updates && __update_queue[5]) ? __l12_next[31:0] : __var[5]));
		__var[6] <= ((__read_request && (__vid == 6)) ? __in : ((__apply_updates && __update_queue[6]) ? __l13_next[31:0] : __var[6]));
		__var[7] <= ((__read_request && (__vid == 7)) ? __in : ((__apply_updates && __update_queue[7]) ? __l14_next[31:0] : __var[7]));
		__expr[0] <= ((__read_request && (__vid == 17)) ? __in : __expr[0]);
	end 
	always @* case (__vid)
		8: __out = __there_are_updates;
		11: __out = __task_id[0];
		14: __out = __done;
		15: __out = __open_loop;
		16: __out = __state[0];
		default: __out = ((__vid < 17) ? __var[__vid] : __expr[(__vid - 17)]);
	endcase
	assign __wait = (__open_loop_tick || (__any_triggers || __continue));
endmodule

module program_logic(
  input wire clk,
  input wire reset,

  input wire[15:0]  s0_address,
  input wire        s0_read,
  input wire        s0_write,

  output reg [31:0] s0_readdata,
  input  wire[31:0] s0_writedata,

  output reg        s0_waitrequest
);
  // Unpack address into module id and variable id
  wire [1:0] __mid = s0_address[13:12];
  wire[11:0] __vid = s0_address[11: 0];
  // Module Instantiations:
  wire[31:0] m0_out;
  wire       m0_wait;
  M0 m0(
    .__clk(clk),
    .__read((__mid == 0) & s0_write),
    .__vid(__vid),
    .__in(s0_writedata),
    .__out(m0_out),
    .__wait(m0_wait)
  );
  // Output Demuxing:
  reg[31:0] rd;
  reg wr;
  always @(*) begin
    case (__mid)
      0: begin rd = m0_out; wr = m0_wait; end
      default: begin rd = 0; wr = 0; end
    endcase
  end
  // Output Logic:
  always @(posedge clk) begin
    s0_waitrequest <= (s0_read | s0_write) ? wr : 1'b1;
    s0_readdata <= rd;
  end
endmodule
