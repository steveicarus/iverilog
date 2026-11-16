// Check two-state conversion after constant-function division and modulo assignments.

module test;

  function [31:0] calc(input [7:0] divisor);
    bit [7:0] quotient, remainder;
    logic [7:0] four_quotient, four_remainder;

    quotient = 8'hc3;
    remainder = 8'hc3;
    four_quotient = 8'hc3;
    four_remainder = 8'hc3;
    quotient /= divisor;
    remainder %= divisor;
    four_quotient /= divisor;
    four_remainder %= divisor;
    calc = {quotient, remainder, four_quotient, four_remainder};
  endfunction

  localparam [31:0] ZERO = calc(0);
  localparam [31:0] NONZERO = calc(2);

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(ZERO, 32'h0000xxxx);
    `check(NONZERO, 32'h61016101);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
