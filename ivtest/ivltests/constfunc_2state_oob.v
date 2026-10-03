// Check out-of-bounds bit and part selects in constant functions.

module test;

  function [9:0] f(input integer index);
    bit [3:0] v2;
    logic [3:0] v4;
    v2 = 4'b1010;
    v4 = 4'b1010;
    f = {v2[index], v2[index +: 4], v4[index], v4[index +: 4]};
  endfunction

  localparam [9:0] INSIDE = f(0);
  localparam [9:0] PARTIAL_HIGH = f(2);
  localparam [9:0] LAST_BIT = f(3);
  localparam [9:0] PARTIAL_LOW = f(-2);
  localparam [9:0] HIGH = f(4);
  localparam [9:0] LOW = f(-4);

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(INSIDE, 10'b01010_01010);
    `check(PARTIAL_HIGH, 10'b00010_0xx10);
    `check(LAST_BIT, 10'b10001_1xxx1);
    `check(PARTIAL_LOW, 10'b01000_x10xx);
    `check(HIGH, 10'b00000_xxxxx);
    `check(LOW, 10'b00000_xxxxx);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
