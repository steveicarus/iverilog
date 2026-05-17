// Check that continuous assignments to unpacked arrays preserve delay.

module test;

  reg failed;

  wire delayed[0:1];
  reg value[0:1];

  assign #5 delayed = value;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    value[0] = 1'b1;
    value[1] = 1'b0;

    #5;
    `check(delayed[0], 1'b1)
    `check(delayed[1], 1'b0)

    value[0] = 1'b0;
    value[1] = 1'b1;

    #4;
    `check(delayed[0], 1'b1)
    `check(delayed[1], 1'b0)

    #1;
    `check(delayed[0], 1'b0)
    `check(delayed[1], 1'b1)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
