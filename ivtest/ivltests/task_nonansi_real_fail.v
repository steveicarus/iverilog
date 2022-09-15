// Check that it is an error to declare a non-ANSI task port with implicit
// packed dimensions if it is later redeclared as a real typed variable.

module test;

  task t;
    input [3:0] x;
    real x;
    $display("FAILED");
  endtask

  initial t(10);

endmodule
