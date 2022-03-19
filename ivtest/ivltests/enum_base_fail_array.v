// Check that using an array type as the base type for an enum results in an
// error.

module test;

  typedef logic T[1:0];

  enum T {
    A
  } e;

  initial begin
    $display("FAILED");
  end

endmodule
