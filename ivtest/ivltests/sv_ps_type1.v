// Check that it is possible to reference a package scoped type identifier, even if
// there is a identifier of the same name in the current scope.

bit failed = 1'b0;

`define check(expr, val) \
  if (expr !== val) begin \
    $display("FAILED: %s, expected %0d, got %0d", `"expr`", val, expr); \
    failed = 1'b1; \
  end

package P;
  typedef logic [31:0] T;
endpackage

module test;
  logic T;
  initial begin
    P::T x;
    `check($bits(x), 32)

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
