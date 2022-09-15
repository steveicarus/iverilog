// Check that it is an error to declare a non-ANSI module port with implicit
// packed dimensions if it is later redeclared as a enum typed variable. Even if
// the size of the packed dimensions matches that of the size of the enum type.

typedef enum integer {
  A, B
} T;

module test(x);
  output [31:0] x;
  T x;

  initial begin
    $display("FAILED");
  end

endmodule
