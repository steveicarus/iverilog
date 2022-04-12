// Check that it is an error to declare a non-ANSI task port with implicit
// packed dimensions if it is later redeclared as a packed struct typed
// variable. Even if the size of the packed dimensions matches that of the size
// of the struct.

typedef struct packed {
  reg [31:0] x;
  reg [7:0] y;
} T;

module test;

  task t;
    input [47:0] x;
    T x;
    $display("FAILED");
  endtask

  initial t(10);

endmodule
