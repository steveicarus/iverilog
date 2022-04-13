// Check that part selects are evaluated correctly within assignment patterns.
// The result should be the same as assigning the expression to a variable with
// the same type as the base type of the assignment pattern target.

module test;

  int d[];
  int tmp;

  bit failed = 1'b0;

  `define check(expr) \
    d = '{expr}; \
    tmp = expr; \
    if (d[0] !== tmp) begin \
      $display("FAILED: `%s`, got %0d, expected %0d", `"expr`", d[0], tmp); \
      failed = 1'b1; \
    end

  logic [23:0] x;
  int i = 13;

  initial begin
    x = 24'ha5a5a5;
    `check(x[0])
    `check(x[7:0])
    `check(x[3+:8])
    `check(x[23-:5])
    `check(x[i+:8])
    `check(x[i-:5])

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
