// Check that declaring an integer typed variabe for a signal that was
// previously declared as a non-ANSI task port is an error. Even if the types
// for both declarations are the same.

module test;

  task t;
    input integer x;
    integer x;
    $display("FAILED");
  endtask

  initial t();

endmodule
