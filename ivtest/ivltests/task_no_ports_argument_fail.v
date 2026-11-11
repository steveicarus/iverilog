// Check that a static task without ports rejects an actual argument.

module test;
  task t;
    $display("FAILED");
  endtask

  initial t(7); // Error: The task has no ports to bind the argument to.
endmodule
