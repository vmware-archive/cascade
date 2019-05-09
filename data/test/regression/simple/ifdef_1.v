initial begin

  `define wow
  `define nest_one 
  `define second_nest 
  `define nest_two

  `ifdef wow
    $write("1"); 
    $write("2");
    `ifdef nest_one
      $write("3"); 
      `ifdef nest_two
        $write("4"); 
      `else
        $write("0"); 
      `endif
    `else
      $write("0"); 
    `endif 
    $write("5");
    `ifndef foo
      `ifndef bar
        $write("6");
      `endif
    `endif
  `else
    $write("0"); 
    `ifdef second_nest
      $write("0"); 
    `else
      $write("0"); 
    `endif
  `endif

  `ifdef foo
    $write("0");
  `else
    `ifndef bar
      $write("7");
    `endif
  `endif

  $finish;
end
