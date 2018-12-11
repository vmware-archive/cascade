module Array();

  parameter W = 2;
  parameter FINISH = 1;

  localparam N = (1 << W) - 1;
  localparam K = 3*W;

  reg[W-1:0] low;
  reg[K:0]   mid  [N:0];    
  reg        high [1:0][1:0][1:0];

  reg[63:0] COUNT = 0;
  always @(posedge clock.val) begin
    low <= low + 1;
    mid[low] <= mid[low] + 1; 
    high[mid[N][K]][mid[N][1]][mid[N][0]] <= high[mid[N][K]][mid[N][1]][mid[N][0]] + 1;

    if (high[1][0][0]) begin
      $display(COUNT);
      $finish(FINISH); 
    end else
      COUNT <= COUNT + 1;
  end

endmodule
