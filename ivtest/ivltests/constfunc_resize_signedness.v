// Check that constant-function resizing preserves expression signedness.

module test;

  reg failed;

  function [7:0] calc;
    input signed [6:0] a;
    input signed [7:0] b;
    input integer op;
    case (op)
      0: calc = a / b;
      1: calc = a % b;
      2: calc = a < b;
      3: calc = a >= b;
      4: calc = a >>> 1;
      5: calc = a[6:0] >>> 1;
    endcase
  endfunction

  function [128:0] wide_shift;
    input signed [64:0] a;
    wide_shift = a >>> 1;
  endfunction

  localparam [7:0] quotient = calc(-7'sd7, 8'sd3, 0);
  localparam [7:0] remainder = calc(-7'sd7, 8'sd3, 1);
  localparam [7:0] less = calc(-7'sd7, 8'sd3, 2);
  localparam [7:0] greater_equal = calc(-7'sd7, 8'sd3, 3);
  localparam [7:0] shifted = calc(-7'sd7, 8'sd3, 4);
  localparam [7:0] selected = calc(-7'sd7, 8'sd3, 5);
  localparam [128:0] wide = wide_shift(-65'sd1);

  `define check(val, exp) \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    `check(quotient, -8'd2);
    `check(remainder, -8'd1);
    `check(less, 8'd1);
    `check(greater_equal, 8'd0);
    `check(shifted, -8'd4);
    `check(selected, 8'h3c);
    `check(wide, {129{1'b1}});

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
