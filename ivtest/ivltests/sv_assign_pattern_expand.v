// Check that the width of the element type of the target of an assignment
// pattern is considered when evaluating the expression. The result should be
// the same as assigning the expression to a variable with the same type as the
// base type of the assignment pattern target.

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

  shortint x;
  bit [7:0] y;

  initial begin
    x = -11;
    y = 8'h80;

    // Sign extension
    `check(x)
    `check(16'sh8000)

    // No sign extension
    `check(8'hff)
    `check(y)

    // Gets evaluated in a 32 bit context, the result is 2/-2 not 0
    `check(1'b1 + 1'b1)
    `check(1'sb1 + 1'sb1)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
