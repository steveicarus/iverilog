// Check that it is possible to reference a package scoped function, even if
// there is a type identifier of the same name in the current scope.

bit failed = 1'b0;

`define check(expr, val) \
  if (expr !== val) begin \
    $display("FAILED: %s, expected %0d, got %0d", `"expr`", val, expr); \
    failed = 1'b1; \
  end

package P;
  function integer T(integer x);
    return x + 1;
  endfunction
endpackage

module test;
  typedef integer T;
  initial begin
    integer x;

    x = P::T(10);
    `check(x, 11)

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
