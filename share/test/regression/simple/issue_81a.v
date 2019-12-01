module foo(input wire clk);
  integer COUNT = 0;
  always @(posedge clk) begin
    COUNT <= COUNT + 1;
    if (COUNT == 4) begin
      $finish;
    end
    $write(COUNT);
  end
endmodule

// The only place that clock.val is referenced is inside of a module
// instantiation. Prior to issue 81, cascade was ignoring instantiation
// arguments as external sources of reads and writes.
foo f(.clk(clock.val));
