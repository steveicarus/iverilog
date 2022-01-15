// Verify that the width is only propagated for a vector multiply.
// The second (real valued) multiply should not set the expression
// width to 1.
module top;
  integer Ival = 14;
  integer result;

  initial begin
    result = Ival * 216 * 140e-3;
    if (result !== 423) $display("Failed:, expected 423, got %0d", result);
    else $display("PASSED");
  end
endmodule
