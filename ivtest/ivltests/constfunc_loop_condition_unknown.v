// Check unknown loop conditions in constant functions and at run time.

module test;

  function [2:0] f(input [1:0] value);
    reg [1:0] condition;
    integer count;
    begin
      f = 3'b000;
      condition = value;
      while (condition) begin
        f[0] = 1'b1;
        condition = 0;
      end

      condition = value;
      for (count = 0; condition; count = count + 1) begin
        condition = 0;
      end
      f[1] = (count == 1);

      condition = value;
      count = 0;
      do begin
        count = count + 1;
        if (count == 2) condition = 0;
      end while (condition);
      f[2] = (count == 2);
    end
  endfunction

  localparam [2:0] ONE_X = f(2'b1x);
  localparam [2:0] ONE_Z = f(2'b1z);
  localparam [2:0] ZERO_X = f(2'b0x);
  localparam [2:0] ZERO_Z = f(2'b0z);

  reg [1:0] value;
  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(ONE_X, 3'b111);
    value = 2'b1x;
    `check(f(value), 3'b111);
    `check(ONE_Z, 3'b111);
    value = 2'b1z;
    `check(f(value), 3'b111);
    `check(ZERO_X, 3'b000);
    value = 2'b0x;
    `check(f(value), 3'b000);
    `check(ZERO_Z, 3'b000);
    value = 2'b0z;
    `check(f(value), 3'b000);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
