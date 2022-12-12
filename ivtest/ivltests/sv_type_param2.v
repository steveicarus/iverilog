// Check that it is possible to declare multiple type parameters as a list with
// a single type keyword.

bit failed = 1'b0;

`define check(expr, val) \
  if (expr != val) begin \
    $display("failed: %s, expected %0d, got %0d", `"expr`", val, expr); \
    failed = 1'b1; \
  end

module test #(
  parameter type T1 = integer, T2 = real
);
  T1 x;
  T2 y = 1.23;

  initial begin
    `check($bits(x), $bits(integer))
    `check($bits(T1), $bits(integer))
    `check(y, 1.23)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
