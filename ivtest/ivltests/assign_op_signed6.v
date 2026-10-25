// Check unsigned compound operations on full-width selects and single-element
// concatenations of signed vectors. Selecting the only bit of a signed vector
// must make >>>= shift in zeroes instead of extending the sign bit.

module test;

  bit failed = 1'b0;
  logic signed [3:0] value;
  logic signed [0:0] one_bit;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    value = -6;
    value /= 4'sd3;
    `check(value, -4'sd2);

    value = -6;
    value[3:0] /= 4'sd3;
    `check(value, 4'd3);

    value = -6;
    value[0 +: 4] >>>= 4'd1;
    `check(value, 4'd5);

    value = -6;
    value[3 -: 4] >>>= 4'd1;
    `check(value, 4'd5);

    value = -6;
    {value} /= 4'sd3;
    `check(value, 4'd3);

    one_bit = 1'b1;
    one_bit >>>= 1'd1;
    `check(one_bit, 1'b1);

    one_bit = 1'b1;
    one_bit[0] >>>= 1'd1;
    `check(one_bit, 1'b0);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
