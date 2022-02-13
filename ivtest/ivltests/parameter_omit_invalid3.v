// Check that declaring changing the parameter type to an implicit type without
// the `parameter` keyword results in an error.

module test #(parameter real A = 1.0, signed B = 2);
  initial begin
    $display("FAILED");
  end
endmodule
