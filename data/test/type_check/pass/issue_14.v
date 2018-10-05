localparam x = 0;
if (x == 1) begin
end else if (x == 2) begin
end else if (x == 3) begin
end else begin
  wire m;
end

if (x == 0) begin
  if (x == 0) begin
    wire n;
  end
end 
else ;

if (x == 0) 
  if (x == 0)
    wire q;
  else ;
else ;

wire a = genblk1.m;
wire b = genblk2.genblk1.n;
wire c = genblk3.q;

initial $finish;
