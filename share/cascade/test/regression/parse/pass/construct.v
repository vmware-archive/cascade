reg clk, w, x, y, z;

module foo();
  //initial fork
  //join
  
  initial begin
  end

  //always fork
  //join

  //always begin
  //end

  always @* ;

  always @* begin
  end
  
  always @(*) begin
  end

  always @ (posedge clk) begin
  end

  always @ (negedge clk) begin
  end

  always @ (posedge clk, x, y, z) begin
  end

  always @ (x or w, y or z) begin
  end
endmodule
