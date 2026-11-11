// Check that a task output argument must be a procedural l-value.

module test;
  task t;
    output value;
    value = 1'b1;
  endtask

  initial begin
    t(1 + 2); // Error: An arithmetic expression is not a procedural l-value.
    $display("FAILED");
  end
endmodule
