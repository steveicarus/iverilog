// Check two-state conversion of compound assignment results in a packed union.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  union packed {
    logic [3:0] four;
    bit [3:0] two;
  } value;

  initial begin
    value.four = 4'h5;
    value.two /= 0;
    `check(value.four, 4'h0);

    value.four = 4'h5;
    value.four /= 0;
    `check(value.four, 4'bxxxx);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
