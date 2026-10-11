// Check that invalid array-query results keep their signed integer type.

module test;

  reg [7:0] values [0:1];
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    // Dimension 0 does not exist.
    `check($left(values, 0) >>> 8, 32'bx);
    `check($right(values, 0) >>> 8, 32'bx);
    `check($low(values, 0) >>> 8, 32'bx);
    `check($high(values, 0) >>> 8, 32'bx);
    `check($increment(values, 0) >>> 8, 32'bx);
    `check($size(values, 0) >>> 8, 32'bx);

    // Dimension 3 does not exist.
    `check($left(values, 3) >>> 8, 32'bx);
    `check($right(values, 3) >>> 8, 32'bx);
    `check($low(values, 3) >>> 8, 32'bx);
    `check($high(values, 3) >>> 8, 32'bx);
    `check($increment(values, 3) >>> 8, 32'bx);
    `check($size(values, 3) >>> 8, 32'bx);

    `check($increment(values, 1) >>> 8, -1);
    `check($increment(values, 2) >>> 8, 0);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
