// Check that it is possible to declare the data type for an integer type task
// port before the direction for non-ANSI style port declarations.

module test;

  task t;
    integer x;
    input x;
    if (x == 10 && $bits(x) == $bits(integer)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask

  initial t(10);

endmodule
