// Check that an error is reported trying to provide a positional argument to a
// task after a named argument.

module test;

  task t(integer a, integer b);
    $display("FAILED");
  endtask

  initial begin
    t(.a(2), 1); // This should fail. Positional arguments must precede
                 // named arguments.
  end

endmodule
