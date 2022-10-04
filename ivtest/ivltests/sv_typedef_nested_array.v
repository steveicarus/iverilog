// Check that nested unpacked array types are supported.

module test;

  localparam A = 3;
  localparam B = 5;
  localparam C = 2;
  localparam D = 7;

  typedef logic [31:0] T1[A];
  typedef T1 T2[B][C];

  T2 x[D];
  T2 y;

  bit failed = 1'b0;

  `define check(expr, val) \
    if (expr !== val) begin \
      $display("FAILED: %s, expected %0d, got %0d", `"expr`", val, expr); \
      failed = 1'b1; \
    end

  initial begin
    `check($unpacked_dimensions(x), 4)
    `check($size(x, 1), A)
    `check($size(x, 2), B)
    `check($size(x, 3), C)
    `check($size(x, 4), D)

    `check($unpacked_dimensions(y), 3)
    `check($size(y, 1), A)
    `check($size(y, 2), B)
    `check($size(y, 3), C)

    `check($bits(T2), $bits(integer) * A * B * C)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
