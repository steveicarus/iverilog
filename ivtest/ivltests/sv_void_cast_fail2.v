// Check that void casting a task results in an error

module test;

  task t(int x);
  endtask

  initial begin
    void'(t(10));
  end

endmodule
