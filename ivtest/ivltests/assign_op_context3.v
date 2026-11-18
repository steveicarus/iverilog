// Check operand widths, signedness, and self-determined shift counts in
// constant-function compound assignments.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function [3:0] divide_literal(input [3:0] x);
    x /= 8'd17;
    divide_literal = x;
  endfunction

  function [3:0] remainder(input [3:0] x, input [7:0] y);
    x %= y;
    remainder = x;
  endfunction

  function signed [3:0] divide_signed(input signed [3:0] x,
                                                input signed [7:0] y);
    x /= y;
    divide_signed = x;
  endfunction

  function [7:0] shift(input [7:0] x, input [3:0] count);
    x <<= (count + 4'd1);
    shift = x;
  endfunction

  localparam [3:0] quotient = divide_literal(12);
  localparam [3:0] rem_value = remainder(12, 17);
  localparam signed [3:0] signed_quotient = divide_signed(-6, 3);
  localparam [7:0] shifted = shift(1, 15);

  initial begin
    `check(quotient, 4'd0);
    `check(rem_value, 4'd12);
    `check(signed_quotient, -4'sd2);
    `check(shifted, 8'd1);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
