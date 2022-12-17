// Check that array query functions return the correct value for C style arrays

module test;

  bit failed = 1'b0;

  `define check(expr, val) \
    if (expr !== val) begin \
      $display("FAILED: %s, expected %0d, got %0d", `"expr`", val, expr); \
      failed = 1'b1; \
    end

  bit [1:0] a[10];

  initial begin
    `check($left(a), 0)
    `check($right(a), 9)
    `check($low(a), 0)
    `check($high(a), 9)
    `check($size(a), 10)
    `check($increment(a), -1)
    `check($dimensions(a), 2)
    `check($unpacked_dimensions(a), 1)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
