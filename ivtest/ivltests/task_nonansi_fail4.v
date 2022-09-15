// Check that declaring an integer typed non-ANSI task port for signal that was
// previously declared as a variable is an error. Even if the types for both
// declarations are the same.

module test;

  task t;
    integer x;
    input integer x;
    $display("FAILED");
  endtask

  integer y;
  initial t(y);

endmodule
