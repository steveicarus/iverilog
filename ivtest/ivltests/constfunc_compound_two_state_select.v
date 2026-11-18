// Check two-state compound assignment conversion for array words and part selects.

module test;

  function [15:0] calc(input [7:0] divisor);
    bit [7:0] value[0:1];

    value[0] = 8'hc3;
    value[1] = 8'hb5;
    value[0] /= divisor;
    value[1][3:0] %= divisor;
    calc = {value[0], value[1]};
  endfunction

  localparam [15:0] ZERO = calc(0);
  localparam [15:0] NONZERO = calc(2);

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(ZERO, 16'h00b0);
    `check(NONZERO, 16'h61b1);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
