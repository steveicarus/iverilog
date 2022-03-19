// Check that using a dynamic array type as the base type for an enum results in
// an error.

module test;

  typedef logic T[];

  enum T {
    A
  } e;

  initial begin
    $display("FAILED");
  end

endmodule
