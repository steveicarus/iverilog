// Check that argument ordering is checked when the task body is empty.

module test;

  task t(integer a, integer b = 0);
  endtask

  initial begin
    t(.a(2), 1); // Error: Positional arguments must precede named arguments.
    $display("FAILED");
  end

endmodule
