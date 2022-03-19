// Check that using an enum type as the base type for an enum results in an
// error.

module test;

  typedef enum {
    X
  } T;

  enum T {
    A
  } e;

  initial begin
    $display("FAILED");
  end

endmodule
