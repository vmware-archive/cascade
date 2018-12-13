module main();
  initial begin 
    fork
      #0  $display(1);
      #10 $display(2);
      #10 $display(3);
    join
    $finish(1);
  end
endmodule

main m();
