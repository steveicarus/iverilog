// Check two-state conversion of concatenation targets in constant functions.

module test;

  function [7:0] convert(input [7:0] value, input reverse);
    bit [3:0] two;
    logic [3:0] four;

    if (reverse) begin
      {four, two} = value;
    end else begin
      {two, four} = value;
    end
    convert = {two, four};
  endfunction

  localparam [7:0] TWO_FIRST = convert(8'bx1z0_z01x, 0);
  localparam [7:0] FOUR_FIRST = convert(8'bz01x_x1z0, 1);

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(TWO_FIRST, 8'b0100_z01x);
    `check(FOUR_FIRST, 8'b0100_z01x);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
