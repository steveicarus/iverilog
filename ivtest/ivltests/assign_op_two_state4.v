// Check two-state conversion of compound assignments to packed member selects.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  struct packed {
    logic [3:0] guard;
    struct packed {
      logic signed [3:0] four;
      bit signed [3:0] two;
    } inner;
  } value;

  initial begin
    value = 12'ha5f;
    value.inner.two[1:0] /= 0;
    `check(value, 12'ha5c);

    value = 12'ha5f;
    value.inner.two[0] /= 0;
    `check(value, 12'ha5e);

    value = 12'ha5f;
    value.inner.four[1:0] /= 0;
    `check(value, 12'b101001xx1111);

    value = 12'haff;
    value.inner.two[3:0] >>>= 1;
    value.inner.four[3:0] >>>= 1;
    `check(value, 12'ha77);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
