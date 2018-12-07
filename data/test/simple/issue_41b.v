// tests whether both top and non-top words are built correctly
wire signed[68:0] minus_sixteen = -16;
wire signed[68:0] test = minus_sixteen >>> 2;

initial begin
    $write(test);
    $finish;
end
