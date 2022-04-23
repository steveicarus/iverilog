// Check that declaring an integer typed variable for a signal that was previously
// declared as a real typed non-ANSI task port is an error.

module test;

  task t;
    output real x;
    integer x;
    $display("FAILED");
  endtask

  initial t();

endmodule
