// Check that it is possible to declare type parameters when omitting the
// parameter keyword.

bit failed = 1'b0;

`define check(expr, val) \
  if (expr != val) begin \
    $display("failed: %s, expected %0d, got %0d", `"expr`", val, expr); \
    failed = 1'b1; \
  end

module test #(
  type T1 = integer
);
  T1 x;

  initial begin
    `check($bits(x), $bits(integer))
    `check($bits(T1), $bits(integer))

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
