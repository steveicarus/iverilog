// Check that using a struct type as the base type for an enum results in an
// error.

module test;

  typedef struct packed {
    int x;
  } T;

  enum T {
    A
  } e;

  initial begin
    $display("FAILED");
  end

endmodule
