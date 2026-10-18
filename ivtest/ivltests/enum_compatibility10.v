// Check a select of the only enum bit retains an integral assignment context.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  enum logic [0:0] { ZERO, ONE } value;
  enum logic [4:4] { OFFSET_ZERO, OFFSET_ONE } offset;

  initial begin
    value[0] = 1;
    `check(value, 1'b1);
    value[0:0] = 0;
    `check(value, 1'b0);
    value[0 +: 1] = 1;
    `check(value, 1'b1);
    value[0 -: 1] = 0;
    `check(value, 1'b0);
    offset[4] = 1;
    `check(offset, 1'b1);
    value[0] <= 1;
    #1;
    `check(value, 1'b1);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
