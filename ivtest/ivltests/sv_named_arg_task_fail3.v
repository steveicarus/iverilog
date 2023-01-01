// Check that an error is reported when trying to bind an argument by name that
// is also provided as a positional argument.

module test;

  task t(integer a, integer b);
    $display("FAILED");
  endtask

  initial begin
    t(1, .a(2));
  end

endmodule
