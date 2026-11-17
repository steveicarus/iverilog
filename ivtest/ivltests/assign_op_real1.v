// Check that mixed real/integral compound assignments use real arithmetic.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  `define check_real(val, exp) \
    if (val != exp) begin \
      $display("FAILED(%0d). '%s' expected %f, got %f", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  integer value;
  real fraction;
  real real_value;
  logic signed [3:0] negative;
  logic [7:0] word;
  logic [3:0] hi, lo;
  bit [3:0] two;
  struct packed {
    logic signed [3:0] s;
    logic [3:0] u;
  } members;

  initial begin
    fraction = 0.5;

    // Round the real result when assigning it to an integer target.
    value = 3;
    value *= fraction;
    `check(value, 2);
    value = 3;
    value -= fraction;
    `check(value, 3);
    value = -3;
    value += fraction;
    `check(value, -3);
    value = -3;
    value /= fraction;
    `check(value, -6);

    // Distinguish signed variables from unsigned selects and concatenations.
    negative = -3;
    negative *= fraction;
    `check(negative, -4'sd2);
    negative = -3;
    negative[3:0] *= fraction;
    `check(negative, 4'd7);
    negative = -3;
    {negative} *= fraction;
    `check(negative, 4'd7);
    word = 8'h35;
    word[7:4] *= fraction;
    `check(word, 8'h25);
    {hi, lo} = 8'h31;
    {hi, lo} *= fraction;
    `check({hi, lo}, 8'h19);

    // Convert real results for two-state targets.
    two = 3;
    two *= fraction;
    `check(two, 4'd2);

    // Packed members retain their declared signedness.
    members.s = -3;
    members.u = 13;
    members.s *= fraction;
    members.u *= fraction;
    `check(members.s, -4'sd2);
    `check(members.u, 4'd7);

    // Integral RHS expressions retain their own width and signedness.
    negative = 7;
    real_value = 0.5;
    real_value += (negative + 4'sd1);
    `check_real(real_value, -7.5);
    real_value = 0.5;
    negative = -3;
    real_value *= negative;
    `check_real(real_value, -1.5);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
