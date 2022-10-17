// Check that it is possible to reference a package scoped type identifier in an
// expression, even if there is a non type identifier of the same name in the
// current scope.

bit failed = 1'b0;

`define check(expr, val) \
  if (expr !== val) begin \
    $display("FAILED: %s, expected %0d, got %0d", `"expr`", val, expr); \
    failed = 1'b1; \
  end

package P;
  typedef integer T;
endpackage

module test;
  integer T;
  initial begin
    `check($bits(P::T), $bits(integer))

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
