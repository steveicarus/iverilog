// Check that constant functions ignore packed writes with X/Z indices.

module test;

  function [15:0] f(input [3:0] index);
    reg [7:0] b, p;
    begin
      b = 8'hff;
      p = 8'hff;
      b[index] = 1'b0;
      p[index +: 4] = 4'b0;
      f = {b, p};
    end
  endfunction

  localparam [15:0] UNKNOWN = f(4'bx);
  localparam [15:0] HIGH_IMPEDANCE = f(4'bz);

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    `check(UNKNOWN, 16'hffff);
    `check(HIGH_IMPEDANCE, 16'hffff);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
