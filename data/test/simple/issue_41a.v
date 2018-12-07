wire signed[31:0] minus_one = -1;
wire signed[31:0] left_shifted = minus_one <<< 1;
wire signed[31:0] test = left_shifted >>> 1;

initial begin
    $write(test);
    $finish;
end
