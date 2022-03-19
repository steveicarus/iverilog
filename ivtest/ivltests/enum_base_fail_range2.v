// Check that using a type identifier that resolves to a type with multiple
// packed dimensions as the base type for an enum results in an error.

module test;

  typedef bit [1:0][1:0] T;

  enum T {
    A
  } e;

  initial begin
    $display("FAILED");
  end

endmodule
