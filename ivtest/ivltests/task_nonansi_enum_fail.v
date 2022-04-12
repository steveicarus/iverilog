// Check that it is an error to declare a non-ANSI task port with implicit
// packed dimensions if it is later redeclared as a enum typed variable. Even if
// the size of the packed dimensions matches that of the size of the enum type.

typedef enum integer {
  A, B
} T;

module test;

  task t;
    input [31:0] x;
    T x;
    $display("FAILED");
  endtask

  initial t(10);

endmodule
