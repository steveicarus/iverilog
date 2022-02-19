// Check that it is possible to reference other parameters in the default value
// of a type parameter.

bit failed = 1'b0;

`define check(expr, val) \
  if (expr != val) begin \
    $display("failed: %s, expected %0d, got %0d", `"expr`", val, expr); \
    failed = 1'b1; \
  end

module test #(
  parameter A = 10,
  parameter type T1 = logic [A-1:0]
);
  T1 x;

  initial begin
    `check($bits(x), A)
    `check($bits(T1), A)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
