// Check that using a string type as the base type for an enum results in an
// error.

module test;

  typedef string T;

  enum T {
    A
  } e;

  initial begin
    $display("FAILED");
  end

endmodule
