module waits();
  always begin
    wait (cond) begin
    end
    wait (15);
  end
endmodule
