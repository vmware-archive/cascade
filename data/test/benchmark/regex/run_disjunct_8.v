wire[7:0] char;
wire empty;
reg[31:0] count = 0;
reg done = 0;

(*__target="sw",__file="data/test/benchmark/regex/iliad.hex",__count=8*)
Fifo#(1,8) fifo(
  .clock(clock.val),
  .rreq(!empty),
  .rdata(char),
  .empty(empty)
);

always @(posedge clock.val) begin
  if (empty) begin
    done <= 1;
  end
  if (done) begin
    $write(count);
    $finish;
  end
end

reg[31:0] state = 0;
reg[31:0] i = 0;
reg[31:0] ie = 0;

always @(posedge clock.val) begin
  if (state > 0) begin
    ie <= ie + 1;
  end
  case (state)
    32'd0:
      state <= 1;
    32'd1: case(char) 
      8'h41: begin
        state <= 5;
      end
      8'h54: begin
        state <= 7;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd2: case(char) 
      8'h45: begin
        state <= 8;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd3: case(char) 
      8'h45: begin
        state <= 4;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd4: case(char) 
      8'h20: begin
        state <= 2;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd5: case(char) 
      8'h63: begin
        state <= 11;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd6: case(char) 
      8'h6c: begin
        state <= 14;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd7: case(char) 
      8'h48: begin
        state <= 3;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd8: case(char) 
      8'h4e: begin
        state <= 9;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd9: case(char) 
      8'h44: begin
        //$display("Match %d:%d", i, ie);
        i <= ie + 1;
        count <= count + 1;
        state <= 1;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd10: case(char) 
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd11: case(char) 
      8'h68: begin
        state <= 12;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd12: case(char) 
      8'h69: begin
        state <= 13;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd13: case(char) 
      8'h6c: begin
        state <= 6;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd14: case(char) 
      8'h65: begin
        state <= 15;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd15: case(char) 
      8'h73: begin
        //$display("Match %d:%d", i, ie);
        i <= ie + 1;
        count <= count + 1;
        state <= 1;
      end
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    32'd16: case(char) 
      default: begin
        i <= ie + 1;
        state <= 1;
      end
    endcase
    default: begin
      $display("Unrecognized state!");
      $finish;
    end
  endcase
end
