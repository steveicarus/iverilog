// Check that unary real minus preserves NaN bits.

module test;

  reg failed;
  real nan;
  real result;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    nan = $bitstoreal(64'h7ff8000000000001);

    result = -nan;
    `check($realtobits(result), 64'hfff8000000000001);

    result = -result;
    `check($realtobits(result), 64'h7ff8000000000001);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
