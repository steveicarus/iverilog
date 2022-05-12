// Check that parameters with a scalar type are handled correctly

module test #(
  // This should get truncated to 1'b1
  parameter bit P = 2'b11
);

  bit failed = 1'b0;

  `define check(expr, val) \
    if (expr !== val) begin \
      $display("FAILED: `%s`, expected %0d, got %0d", `"expr`", val, expr); \
      failed = 1'b1; \
    end

  initial begin
    `check($bits(P), 1);
    `check(P + 1'b1, 1'b0);
    `check(P, 1'b1);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
