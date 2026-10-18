// Check signed division and arithmetic right shift assignments to class properties.
// Unsigned divisors must make division unsigned. Both >>= on signed properties and
// >>>= on unsigned properties must shift in zeroes.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  class C;
    logic signed [3:0] value;
    logic [3:0] unsigned_value;
  endclass

  C c;

  initial begin
    c = new;
    c.value = -6;
    c.value /= 4'sd3;
    `check(c.value, -4'sd2);
    c.value >>>= 4'd1;
    `check(c.value, -4'sd1);

    c.value = -6;
    c.value /= 4'd3;
    `check(c.value, 4'd3);
    c.value = -6;
    c.value >>= 4'd1;
    `check(c.value, 4'd5);
    c.unsigned_value = 10;
    c.unsigned_value >>>= 4'd1;
    `check(c.unsigned_value, 4'd5);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
