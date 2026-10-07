// Check real loop conditions in constant functions and at run time.

module test;

  function [2:0] f(input real value);
    real condition;
    integer count;
    begin
      f = 3'b000;
      condition = value;
      while (condition) begin
        f[0] = 1'b1;
        condition = 0.0;
      end

      condition = value;
      for (count = 0; condition; count = count + 1) begin
        condition = 0.0;
      end
      f[1] = (count == 1);

      condition = value;
      count = 0;
      do begin
        count = count + 1;
        if (count == 2) condition = 0.0;
      end while (condition);
      f[2] = (count == 2);
    end
  endfunction

  localparam [2:0] ZERO = f(0.0);
  localparam [2:0] FRACTION = f(0.25);
  localparam [2:0] NEGATIVE = f(-0.25);
  localparam [2:0] INTEGER = f(2.0);

  real value;
  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(ZERO, 3'b000);
    value = 0.0;
    `check(f(value), 3'b000);
    `check(FRACTION, 3'b111);
    value = 0.25;
    `check(f(value), 3'b111);
    `check(NEGATIVE, 3'b111);
    value = -0.25;
    `check(f(value), 3'b111);
    `check(INTEGER, 3'b111);
    value = 2.0;
    `check(f(value), 3'b111);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
