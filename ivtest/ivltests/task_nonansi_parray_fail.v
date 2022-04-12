// Check that it is an error to declare a non-ANSI task port with implicit
// packed dimensions if it is later redeclared as a packed array typed variable.
// Even if the size of the packed dimensions matches that of the size of the
// packed array.

typedef reg [7:0] T1;
typedef T1 [3:0] T2;

module test;

  task t;
    input [31:0] x;
    T2 x;
    $display("FAILED");
  endtask

  initial t(10);

endmodule
