// Check that multi-dimensional real arrays are supported

module test;

  reg failed = 1'b0;

  `define check(expr, val) \
    if (expr != val) begin \
      $display("FAILED: `%s`, expected %f, got %f", `"expr`", val, expr); \
      failed = 1'b1; \
    end

  real r[3:0][1:0];
  integer i;

  initial begin
    for (i = 0; i < 8; i = i + 1) begin
      r[i/2][i%2] = i / 8.0 - 0.5;
    end

    `check(r[0][0], -0.5);
    `check(r[0][1], -0.375);
    `check(r[1][0], -0.25);
    `check(r[1][1], -0.125);
    `check(r[2][0], 0.0);
    `check(r[2][1], 0.125);
    `check(r[3][0], 0.25);
    `check(r[3][1], 0.375);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
