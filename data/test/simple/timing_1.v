module main();
  initial begin
    #0  $display(1);
    #10 $display(2);
    #10 $display(3);
    $finish(1);
  end
endmodule

main m();
