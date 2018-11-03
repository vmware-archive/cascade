localparam W = 10;
localparam N = (1<<W)-1;

reg[W-1:0] low;
reg[2*W:0] mid [N:0];    
reg        high[1:0][N:0];

reg[63:0] COUNT = 0;
always @(posedge clock.val) begin
  low <= low + 1;
  mid[low] <= mid[low] + 1; 
  high[mid[N][2*W]][mid[N][W-1:0]] <= high[mid[N][2*W]][mid[N][W-1:0]] + 1;

  if (high[1][0]) begin
    $display(COUNT);
    $finish;
  end else
    COUNT <= COUNT + 1;
end
