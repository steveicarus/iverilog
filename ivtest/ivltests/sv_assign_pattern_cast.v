// Check that implicit cast works for expressions in assignment patterns. The
// result should be the same as assigning the expression to a variable with the
// same type as the base type of the assignment pattern target.

module test;

  int dv[];
  real dr[];

  int tmpv;
  real tmpr;

  bit failed = 1'b0;

  `define check_v(expr) \
    dv = '{expr}; \
    tmpv = expr; \
    if (dv[0] !== tmpv) begin \
      $display("FAILED: `%s`, got %0d, expected %0d", `"expr`", dv[0], tmpv); \
      failed = 1'b1; \
    end

  `define check_r(expr) \
    dr = '{expr}; \
    tmpr = expr; \
    if (dr[0] != tmpr) begin \
      $display("FAILED: `%s`, got %0d, expected %0d", `"expr`", dr[0], tmpr); \
      failed = 1'b1; \
    end

  real r;
  int i;

  initial begin
    r = 4.56;
    i = -11;

    // Implicit cast from real to vector
    `check_v(1.234e16)
    `check_v(r)

    // Implicit cast from vector to real
    `check_r(32'hfffffff0)
    `check_r(i)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
