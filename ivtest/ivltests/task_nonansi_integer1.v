// Check that it is possible to declare the data type for an integer type task
// port separately from the direction for non-ANSI style port declarations.

module test;

  task t;
    input x;
    integer x;
    if (x == 10 && $bits(x) == $bits(integer)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask

  initial t(10);

endmodule
