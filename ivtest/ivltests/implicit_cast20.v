// Check two-state conversion of packed members and their selects in blocking assignments.

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
    value.inner.two[1:0] = 2'bxz;
    `check(value, 12'ha5c);

    value = 12'ha5f;
    value.inner.two[0] = 1'bz;
    `check(value, 12'ha5e);

    value = 12'ha5f;
    value.inner.four[1:0] = 2'bxz;
    `check(value, 12'b101001xz1111);

    value = 12'ha5f;
    {value.inner.two[1:0], {value.inner.four[1:0]}} = 4'bxzzx;
    `check(value, 12'b101001zx1100);

    value = 12'ha5f;
    {value.inner.two, value.inner.four} = 8'bx1z0_z01x;
    `check(value, 12'b1010_z01x_0100);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
