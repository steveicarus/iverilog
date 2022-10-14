// Check that it is possible to reference a package scoped identifier, even if
// there is a type identifier of the same name in the current scope.

bit failed = 1'b0;

`define check(expr, val) \
  if (expr !== val) begin \
    $display("FAILED: %s, expected %0d, got %0d", `"expr`", val, expr); \
    failed = 1'b1; \
  end

package P;
  integer T = 10;
endpackage

module test;
  typedef integer T;
  integer x;
  initial begin
    x = P::T;
    `check(x, 10)

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
