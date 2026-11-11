// Check that an empty positional argument cannot follow a named argument.

module test;

  task t(integer a, integer b = 0);
    $display("FAILED");
  endtask

  initial begin
    t(.a(2),); // Error: Positional arguments must precede named arguments.
  end

endmodule
