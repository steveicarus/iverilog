// Check two-state conversion of packed members in blocking assignments.

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
    {value.inner.two, value.inner.four} = 8'bx1z0_z01x;
    `check(value, 12'b1010_z01x_0100);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
