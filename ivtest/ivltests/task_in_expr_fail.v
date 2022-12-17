// Check that an error is reported when a task is used in an expression

module test;

  task t;
  endtask

  initial begin
    int x;
    x = t() + 1; // This should fail, task can not be used in expression
    $display("FAILED");
  end

endmodule
