// Check that using a real type as the base type for an enum results in an
// error.

module test;

  typedef real T;

  enum T {
    A
  } e;

  initial begin
    $display("FAILED");
  end

endmodule
