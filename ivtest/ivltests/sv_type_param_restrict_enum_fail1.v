// Check that an enum restricted type parameter rejects non-enum defaults.

module test #(
  parameter type enum T = int
);

  initial begin
    $display("FAILED");
  end

endmodule
