// Check that resistive MOS outputs preserve the reduced drive strength.

module test;
  reg value;
  wire n, p, c;
  reg [23:0] strength;
  reg failed;

  rnmos (n, value, 1'b1);
  rpmos (p, value, 1'b0);
  rcmos (c, value, 1'b1, 1'b0);

  `define check(val, exp) \
    $sformat(strength, "%v", val); \
    if (strength !== exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %s", `__LINE__, \
               `"val`", exp, strength); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    value = 1'b0;
    #1;
    `check(n, "Pu0");
    `check(p, "Pu0");
    `check(c, "Pu0");

    value = 1'b1;
    #1;
    `check(n, "Pu1");
    `check(p, "Pu1");
    `check(c, "Pu1");

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
