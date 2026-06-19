// Check that unary real minus preserves the sign of zero.

module test;

  reg failed;
  real zero;
  real result;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    zero = 0.0;

    result = -zero;
    `check($realtobits(result), 64'h8000000000000000);

    result = -result;
    `check($realtobits(result), 64'h0000000000000000);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
