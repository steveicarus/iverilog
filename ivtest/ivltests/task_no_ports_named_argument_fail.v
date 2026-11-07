// Check that a static task without ports rejects a named actual argument.

module test;
  task t;
    $display("FAILED");
  endtask

  initial t(.value(7)); // Error: The task has no argument named value.
endmodule
