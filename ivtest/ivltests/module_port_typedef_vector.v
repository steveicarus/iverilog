// Check that a packed array type identifier used for a module port is
// elaborated in the correct scope.

localparam A = 2;

typedef logic [A-1:0] T;

module test (
  input T x
);

  localparam A = 5;

  bit failed = 1'b0;

  `define check(expr, val) \
    if (expr !== val) begin \
      $display("FAILED: %s, expected %0d, got %0d", `"expr`", val, expr); \
      failed = 1'b1; \
    end

  initial begin
    `check($bits(x), 2);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
