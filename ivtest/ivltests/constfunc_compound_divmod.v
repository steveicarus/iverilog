// Check that constant-function /= and %= use unsigned arithmetic when the
// dividend is unsigned, even if the divisor is a signed variable.

module test;

  function [7:0] unsigned_calc(input [3:0] lhs, input signed [3:0] rhs);
    reg [3:0] quotient;
    quotient = lhs;
    quotient /= rhs;
    lhs %= rhs;
    unsigned_calc = {quotient, lhs};
  endfunction

  function [7:0] signed_calc(input signed [3:0] lhs, rhs);
    reg signed [3:0] quotient;
    quotient = lhs;
    quotient /= rhs;
    lhs %= rhs;
    signed_calc = {quotient, lhs};
  endfunction

  localparam [7:0] unsigned_positive = unsigned_calc(4'd8, 4'sd3);
  localparam [7:0] unsigned_negative = unsigned_calc(4'd14, -4'sd3);
  localparam [7:0] signed_positive = signed_calc(-4'sd8, 4'sd3);
  localparam [7:0] signed_negative = signed_calc(-4'sd8, -4'sd3);

  bit failed = 1'b0;
  reg [3:0] lhs;
  reg signed [3:0] rhs;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(unsigned_positive, {4'd2, 4'd2});
    `check(unsigned_negative, {4'd1, 4'd1});
    `check(signed_positive, {-4'sd2, -4'sd2});
    `check(signed_negative, {4'sd2, -4'sd2});

    lhs = 4'd8;
    rhs = 4'sd3;
    `check(unsigned_calc(lhs, rhs), {4'd2, 4'd2});
    `check(signed_calc(lhs, rhs), {-4'sd2, -4'sd2});
    rhs = -4'sd3;
    `check(signed_calc(lhs, rhs), {4'sd2, -4'sd2});
    lhs = 4'd14;
    `check(unsigned_calc(lhs, rhs), {4'd1, 4'd1});

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
