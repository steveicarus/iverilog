// Check that declaring two non-ANSI task ports with an implicit type and the
// same name is an error. Even if the signal was previously declared as an
// variable.

module test;

  task t;
    integer x;
    input x;
    input x;
    $display("FAILED");
  endtask

  integer y;
  initial t(y, y);

endmodule
