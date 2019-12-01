reg[8*14-1:0] s;

initial begin
  s = "Hello world";
  $display("%s is stored as %h", s, s);
  s = {s, "!!!"};
  $display("%s is stored as %h", s, s);
  $finish;
end
