// Check that compound shifts with X/Z counts produce X in constant functions.

module test;

  function [31:0] shift(input signed [7:0] value, input [7:0] count);
    reg signed [7:0] left, arith_left, right, arith_right;
    begin
      left = value;
      arith_left = value;
      right = value;
      arith_right = value;
      left <<= count;
      arith_left <<<= count;
      right >>= count;
      arith_right >>>= count;
      shift = {left, arith_left, right, arith_right};
    end
  endfunction

  localparam [31:0] ALL_X = shift(8'ha5, 8'bx);
  localparam [31:0] ALL_Z = shift(8'ha5, 8'bz);
  localparam [31:0] LOW_X = shift(8'ha5, 8'b0000000x);
  localparam [31:0] LOW_Z = shift(8'ha5, 8'b0000000z);
  localparam [31:0] HIGH_X = shift(8'ha5, 8'bx0000001);
  localparam [31:0] HIGH_Z = shift(8'ha5, 8'bz0000001);
  localparam [31:0] ZERO = shift(8'ha5, 8'd0);
  localparam [31:0] ONE = shift(8'ha5, 8'd1);
  localparam [31:0] LARGE = shift(8'ha5, 8'd8);

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(ALL_X, 32'bx);
    `check(ALL_Z, 32'bx);
    `check(LOW_X, 32'bx);
    `check(LOW_Z, 32'bx);
    `check(HIGH_X, 32'bx);
    `check(HIGH_Z, 32'bx);
    `check(ZERO, 32'ha5a5a5a5);
    `check(ONE, 32'h4a4a52d2);
    `check(LARGE, 32'h000000ff);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
