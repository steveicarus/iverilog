// Check that separate vector assignments preserve each bit's drive strength.

module test;

  reg a, b;
  wire [1:0] value;
  reg [55:0] strength;
  reg failed;

  assign (weak0, weak1) value[1] = a;
  assign (pull0, pull1) value[0] = b;

  `define check(val, exp) \
    $sformat(strength, "%v", val); \
    if (strength !== exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %s", `__LINE__, \
               `"val`", exp, strength); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    a = 1'b0;
    b = 1'b1;
    #1;
    `check(value, "We0_Pu1");

    a = 1'b1;
    b = 1'b0;
    #1;
    `check(value, "We1_Pu0");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
