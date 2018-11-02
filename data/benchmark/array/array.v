localparam W = 10;
localparam N = (1<<W)-1;

reg[  W-1:0] low;
reg[2*W-1:0] mid [N:0];    
reg          high[N:0][N:0];

reg[63:0] COUNT = 0;
always @(posedge clock.val) begin
  // Every cycle low increases by 1
  low <= low + 1;
  // Every 2^W cycles mid[N] increases by 1
  mid[low] <= mid[low] + 1; 
  // It will take 2^W * 2^(2*W) cycles for high[N][N] to go high
  high[mid[N][2*W-1:W]][mid[N][W-1:0]] <= high[mid[N][2*W-1:W]][mid[N][W-1:0]] + 1;

  if (high[N][N]) begin
    $display(COUNT);
    $finish;
  end else
    COUNT <= COUNT + 1;
end
