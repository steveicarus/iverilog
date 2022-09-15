// Check that it is an error to declare a non-ANSI task port with implicit
// packed dimensions if it is later redeclared as a time typed variable. Even if
// the size of the packed dimensions matches that of the size of the time type.

module test;

  task t;
    input [63:0] x;
    time x;
    $display("FAILED");
  endtask

  initial t(10);

endmodule
