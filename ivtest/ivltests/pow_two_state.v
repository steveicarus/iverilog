// Check that power results propagate X and convert to two-state values.

module test;

  typedef bit [7:0] byte_bits;
  localparam byte_bits VALUE = 8'd0 ** -8'sd1;
  bit [7:0] base = 0;
  bit signed [7:0] exponent = -1;
  byte_bits value;
  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(VALUE, 8'h00);
    value = base ** exponent;
    `check(value, 8'h00);
    `check(byte_bits'(base ** exponent), 8'h00);
    `check(base ** exponent, 8'hxx);
    `check((base ** exponent) * 8'd0, 8'hxx);
    `check(8'd0 << (base ** exponent), 8'hxx);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
