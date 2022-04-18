// Check that concatenations are evaluated correctly in assignment patterns.
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

  shortint x;
  byte y;

  initial begin
    x = -1;
    y = 10;
    `check({x,y})
    `check({3{y}})

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
