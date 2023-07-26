// Check that binding a const variable to a task output port fails.

module test;

  const integer x = 10;

  task t(output integer x);
    x = 20;
  endtask

  initial begin
    t(x);
    $display("FAILED");
  end

endmodule
