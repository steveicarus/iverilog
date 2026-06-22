// Check that unary real minus preserves infinities.

module test;

  reg failed;
  real inf;
  real result;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    inf = $bitstoreal(64'h7ff0000000000000);

    result = -inf;
    `check($realtobits(result), 64'hfff0000000000000);

    result = -result;
    `check($realtobits(result), 64'h7ff0000000000000);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
