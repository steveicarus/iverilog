// Check that X/Z packed select indices are out of bounds in constant functions.

module test;

  function [9:0] f(input [3:0] index);
    bit [3:0] v2;
    logic [3:0] v4;
    v2 = 4'b1010;
    v4 = 4'b1010;
    f = {v2[index], v2[index +: 4], v4[index], v4[index +: 4]};
  endfunction

  localparam [9:0] UNKNOWN = f(4'bx);
  localparam [9:0] HIGH_IMPEDANCE = f(4'bz);

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(UNKNOWN, 10'b00000_xxxxx);
    `check(HIGH_IMPEDANCE, 10'b00000_xxxxx);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
