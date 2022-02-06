// Check that using a return value when using the return statement in a task
// results in an error.

module test;

  task t;
    return 10; // This is an error, tasks can not have return values.
  endtask

  initial begin
    t();
    $display("FAILED");
  end

endmodule
