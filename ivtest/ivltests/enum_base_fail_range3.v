// Check that specifying a vector type with multiple packed dimensions as the
// base type for an enum results in an error.

module test;

  enum logic [1:0][1:0] {
    A
  } e;

  initial begin
    $display("FAILED");
  end

endmodule
