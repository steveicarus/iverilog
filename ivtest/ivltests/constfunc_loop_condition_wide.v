// Check wide loop conditions in constant functions and at run time.

module test;

  function [2:0] f(input [95:0] value);
    reg [95:0] condition;
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

  localparam [2:0] ZERO = f(96'd0);
  localparam [2:0] BIT63 = f(96'b1 << 63);
  localparam [2:0] BIT80 = f(96'b1 << 80);
  localparam [2:0] BIT95 = f(96'b1 << 95);

  reg [95:0] value;
  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(ZERO, 3'b000);
    value = 96'd0;
    `check(f(value), 3'b000);
    `check(BIT63, 3'b111);
    value = 96'b1 << 63;
    `check(f(value), 3'b111);
    `check(BIT80, 3'b111);
    value = 96'b1 << 80;
    `check(f(value), 3'b111);
    `check(BIT95, 3'b111);
    value = 96'b1 << 95;
    `check(f(value), 3'b111);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
