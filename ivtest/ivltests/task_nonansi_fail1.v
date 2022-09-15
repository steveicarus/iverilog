// Check that declaring multiple task non-ANSI ports with the same name is an
// error. Even if they both have an implicit type.

module test;

  task t;
    input x;
    input x;
    $display("FAILED");
  endtask

  reg y;
  initial t(y, y);

endmodule
