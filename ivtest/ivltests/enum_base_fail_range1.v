// Check that using a type identifier that resolves to a type with a packed
// dimensions together with another packed dimensions as the base type for an
// enum results in an error.

module test;

  typedef int T;

  enum T [1:0] {
    A
  } e;

  initial begin
    $display("FAILED");
  end

endmodule
