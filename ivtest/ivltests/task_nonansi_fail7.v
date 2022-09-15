// Check that declaring a real typed non-ANSI task port for signal that was
// previously declared as a variable is an error. Even if the types for both
// declarations are the same.

module test;

  task t;
    real x;
    output real x;
    $display("FAILED");
  endtask

  real y;
  initial t(y);

endmodule
