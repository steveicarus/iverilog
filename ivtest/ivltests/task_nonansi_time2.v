// Check that it is possible to declare the data type for a time type task port
// before the direction for non-ANSI style port declarations.

module test;

  task t;
    time x;
    input x;
    if (x == 10 && $bits(x) == $bits(time)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask

  initial t(10);

endmodule
