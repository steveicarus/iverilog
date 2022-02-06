// Check that using a return statment inside a parallel block in a task results
// in an error.

module test;

  task t;
    fork
      // This is an error it is not possible to return from inside a parallel block
      return;
    join
  endtask

  initial begin
    t();
    $display("FAILED");
  end

endmodule
