// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// br_gh1863.v - Regression for GitHub issue #1863
// A 2-state signed vector wider than 64 bits as a class property must
// load, and must hold values that use all of its bits. vvp used to abort
// when it loaded the program.

class C;
  bit signed [64:0] m;
  bit signed [127:0] w;
endclass

module main;
  C c, d;
  bit signed [64:0] t;
  bit signed [127:0] tw;
  bit failed;

  task automatic check65(string name, bit signed [64:0] got,
			 bit signed [64:0] exp);
    if (got !== exp) begin
      $display("FAILED: %s: got %h, expected %h", name, got, exp);
      failed = 1'b1;
    end
  endtask

  initial begin
    failed = 1'b0;
    c = new;

    // Only bit 64 set: the most negative value.
    c.m = 65'h1_0000_0000_0000_0000;
    check65("min", c.m, 65'h1_0000_0000_0000_0000);
    if (!(c.m < 0)) begin
      $display("FAILED: min is not negative");
      failed = 1'b1;
    end

    // Bit 63 set, bit 64 clear: positive, so the sign comes from bit 64.
    c.m = 65'h0_FFFF_FFFF_FFFF_FFFF;
    check65("max64", c.m, 65'h0_FFFF_FFFF_FFFF_FFFF);
    if (!(c.m > 0)) begin
      $display("FAILED: 2^64-1 is not positive");
      failed = 1'b1;
    end

    // Bits on both sides of the 64-bit boundary stay independent.
    c.m = 65'h1_5A5A_5A5A_5A5A_5A5A;
    check65("pattern", c.m, 65'h1_5A5A_5A5A_5A5A_5A5A);
    t = c.m;
    check65("local copy", t, 65'h1_5A5A_5A5A_5A5A_5A5A);
    if (t[64] !== 1'b1 || t[63:0] !== 64'h5A5A_5A5A_5A5A_5A5A) begin
      $display("FAILED: bits of the pattern: %h", t);
      failed = 1'b1;
    end

    // A second object does not share the value.
    d = new;
    d.m = c.m;
    d.m = 65'h0;
    check65("independent", c.m, 65'h1_5A5A_5A5A_5A5A_5A5A);

    // Sign extension into a wider variable.
    c.m = -65'sd1;
    tw = c.m;
    if (tw !== {128{1'b1}}) begin
      $display("FAILED: sign extension: %h", tw);
      failed = 1'b1;
    end

    // A wider property takes the same path.
    c.w = {64'hDEAD_BEEF_0123_4567, 64'h89AB_CDEF_FEDC_BA98};
    if (c.w !== {64'hDEAD_BEEF_0123_4567, 64'h89AB_CDEF_FEDC_BA98}) begin
      $display("FAILED: 128-bit property: %h", c.w);
      failed = 1'b1;
    end

    if (!failed) $display("PASSED");
  end
endmodule
