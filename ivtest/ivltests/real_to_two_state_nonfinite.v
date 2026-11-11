// Check real-to-two-state casts.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  typedef bit [7:0] bits_t;
  real zero = 0.0;
  real value;
  logic [7:0] result;

  initial begin
    value = zero / zero; // NaN.
    result = bits_t'(value);
    `check(result, 8'h00);

    value = 1.0 / zero; // Positive infinity.
    result = bits_t'(value);
    `check(result, 8'h00);

    value = -1.0 / zero; // Negative infinity.
    result = bits_t'(value);
    `check(result, 8'h00);

    value = 1.5;
    result = bits_t'(value);
    `check(result, 8'h02);

    value = -1.5;
    result = bits_t'(value);
    `check(result, 8'hfe);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
