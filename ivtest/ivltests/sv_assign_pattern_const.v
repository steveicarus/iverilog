// Check that named constants can be accessed in assignment patterns. The
// result should be the same as assigning the expression to a variable with the
// same type as the base type of the assignment pattern target.

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

  parameter A = -1;
  localparam B = 10;

  typedef enum {
    X = -1, Y = 0, Z = 1
  } T;

  initial begin
    `check(A)
    `check(B)
    `check(X)
    `check(Y)
    `check(Z)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
