// Check that a static task without ports rejects empty actual arguments.

module test;
  task t;
    $display("FAILED");
  endtask

  initial t(,); // Error: The task has no ports to bind the arguments to.
endmodule
