// Check that it is possible to reference a package scoped type identifier as
// the type in a type cast expression.

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
  integer x;
  initial begin
    x = P::T'(-1024.123);
    `check(x, -1024)

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
