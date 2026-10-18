// Check signed division and arithmetic right shift assignments to dynamic array
// elements. Unsigned divisors must make division unsigned. Both >>= on signed
// elements and >>>= on unsigned elements must shift in zeroes.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  logic signed [3:0] values[];
  logic [3:0] unsigned_values[];
  integer index;

  initial begin
    values = new[1];
    unsigned_values = new[1];
    values[0] = -6;
    values[0] /= 4'sd3;
    `check(values[0], -4'sd2);
    index = 0;
    values[index] >>>= 4'd1;
    `check(values[0], -4'sd1);

    values[0] = -6;
    values[index] /= 4'd3;
    `check(values[0], 4'd3);
    values[0] = -6;
    values[index] >>= 4'd1;
    `check(values[0], 4'd5);
    unsigned_values[0] = 10;
    unsigned_values[index] >>>= 4'd1;
    `check(unsigned_values[0], 4'd5);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
