initial begin
  $write(17.0 == 17.00000);
  $write(1e10 > 17.0);
  $write(1.5e3 > 1.0143121);
  $write(256'd17 == 17.0);
  $write(17.0 == 16'd17); 
  $write(256'd17 <= 17.0);
  $write(17.0 <= 16'd17); 
  $write(256'd16 < 17.0);
  $write(16.0 < 16'd17); 
  $write(256'd16 <= 17.0);
  $write(16.0 <= 16'd17); 
  $finish;
end

