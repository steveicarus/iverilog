// Check that an array type identifier used for a module port is elaborated in
// the correct scope.

localparam A = 2;
localparam B = 4;

typedef logic [A-1:0] T[B];

module test (
  input T x
);

  localparam A = 5;
  localparam B = 10;

  bit failed = 1'b0;

  `define check(expr, val) \
    if (expr !== val) begin \
      $display("FAILED: %s, expected %0d, got %0d", `"expr`", val, expr); \
      failed = 1'b1; \
    end

  initial begin
    `check($size(x, 1), 4);
    `check($size(x, 2), 2);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
