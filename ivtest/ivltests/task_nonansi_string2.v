// Check that it is possible to declare the data type for a string type task
// port before the direction for non-ANSI style port declarations.

module test;

  task t;
    string x;
    input x;
    if (x == "TEST") begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask

  initial t("TEST");

endmodule
