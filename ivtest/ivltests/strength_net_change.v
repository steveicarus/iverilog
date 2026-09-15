// Check that a strength-only change propagates through a strength-aware net.

module test;

  reg enabled;
  wire value, observed;
  reg [23:0] strength;
  reg failed;

  bufif1 (weak0, weak1) (value, 1'b0, 1'b1);
  bufif1 (strong0, strong1) (value, 1'b0, enabled);
  nmos (observed, value, 1'b1);

  `define check(val, exp) \
    $sformat(strength, "%v", val); \
    if (strength !== exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %s", `__LINE__, \
               `"val`", exp, strength); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    enabled = 1'b0;
    #1;
    `check(observed, "We0");

    enabled = 1'b1;
    #1;
    `check(observed, "St0");

    enabled = 1'b0;
    #1;
    `check(observed, "We0");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
