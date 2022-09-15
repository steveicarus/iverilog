// Check that declaring a real typed variable for a signal that was previously
// declared as a non-ANSI task port is an error. Even if the types for both
// declarations are the same.

module test;

  task t;
    output real x;
    real x;
    $display("FAILED");
  endtask

  real y;
  initial t(y);

endmodule
