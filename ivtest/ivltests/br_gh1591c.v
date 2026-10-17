// Check that real %t formatting supports large precisions.

`timescale 1ns/1ps

module test;
  reg [8*1024-1:0] text;
  reg failed;

  `define check(fmt, val, exp) \
    $sformat(text, fmt, val); \
    if (text != exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %0s", `__LINE__, \
               `"val`", exp, text); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    $timeformat(-9, 600, " ns", 0);
    `check("%t", 1.25, {"1.25", {598{"0"}}, " ns"});
    `check("%t", 0.0, {"0.", {600{"0"}}, " ns"});
    `check("%.700t", 1.5, {"1.5", {699{"0"}}, " ns"});
    $timeformat(-12, 600, " ps", 0);
    `check("%t", 1.25, {"1250.", {600{"0"}}, " ps"});

    if (!failed) $display("PASSED");
  end
endmodule
