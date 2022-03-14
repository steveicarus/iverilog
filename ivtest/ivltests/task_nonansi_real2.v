// Check that it is possible to declare the data type for a real type task port
// before the direction for non-ANSI style port declarations.

module test;

  task t;
    real x;
    input x;
    if (x == 1.23) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask

  initial t(1.23);

endmodule
