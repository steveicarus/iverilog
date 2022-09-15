// Check that declaring a non-ANSI task port with an explicit type for a signal
// that was previously declared real variable is an error.

module test;

  task t;
    real x;
    output integer x;
    $display("FAILED");
  endtask

  real y;
  initial t(y);

endmodule
