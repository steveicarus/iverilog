// Check that it is possible to declare the data type for an atom2 type task
// port separately from the direction for non-ANSI style port declarations.

module test;

  task t;
    input x;
    int x;
    if (x == 10 && $bits(x) == $bits(int)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask

  initial t(10);

endmodule
