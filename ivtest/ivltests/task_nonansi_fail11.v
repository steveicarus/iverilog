// Check that declaring two non-ANSI output task ports with an explicit type is
// an error. Even if the types are the same.

module test;

  task t;
    input integer x;
    input integer x;
    $display("FAILED");
  endtask

  integer y;
  initial t(y, y);

endmodule
