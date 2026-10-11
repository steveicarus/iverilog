// Check that constant-function signal reads use the expression signedness.

module test;

  function [7:0] shifted(input signed [7:0] value);
    shifted = (value >>> 2) + 8'b1;
  endfunction

  function [7:0] divided(input signed [7:0] value);
    divided = (value / 8'sd2) + 8'b1;
  endfunction

  function [7:0] signed_shift(input signed [7:0] value);
    signed_shift = value >>> 2;
  endfunction

  localparam [7:0] SHIFTED = shifted(-8'sd2);
  localparam [7:0] DIVIDED = divided(-8'sd2);
  localparam [7:0] SIGNED_SHIFT = signed_shift(-8'sd2);

  reg signed [7:0] value;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    value = -8'sd2;
    `check(SHIFTED, 8'h40);
    `check(DIVIDED, 8'h80);
    `check(SIGNED_SHIFT, 8'hff);
    `check(shifted(value), 8'h40);
    `check(divided(value), 8'h80);
    `check(signed_shift(value), 8'hff);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
