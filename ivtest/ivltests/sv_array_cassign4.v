// Check that continuous assignment of two compatible arrays is supported, even
// if the element types are not identical and one is a built-in integer and the
// other a equivalent packed type.

module test;

  wire signed [31:0] x[1:0];
  wire integer y[1:0];

  assign x = y;

  initial begin
    $display("PASSED");
  end

endmodule
