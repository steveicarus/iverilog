// Check that it is an error to declare a non-ANSI task port with implicit
// packed dimensions if it is later redeclared as an atom2 typed variable. Even
// if the size of the packed dimensions matches that of the size of the atom2
// type.

module test;

  task t;
    input [15:0] x;
    shortint x;
    $display("FAILED");
  endtask

  initial t(10);

endmodule
