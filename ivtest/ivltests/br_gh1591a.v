// Check that %t can format integral values wider than the time type.

`timescale 1ns/1ps

module test;
  reg [2047:0] value;
  reg [8*617-1:0] decimal;
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
    value = {2048{1'b1}};
    $sformat(decimal, "%0d", value);

    `check("%t", value, {decimal, "000"});
    $timeformat(-9, 0, " ns", 0);
    `check("%0t", value, {decimal, " ns"});
    `check("%T", value, {decimal, " ns"});
    $timeformat(-12, 4, " ps", 0);
    `check("%t", value, {decimal, "000.0000 ps"});
    $timeformat(-6, 0, " us", 0);
    `check("%t", value, {decimal[8*617-1:8*3], " us"});

    $timeformat(-9, 0, "", 0);
    `check("%0700t", value, {{83{"0"}}, decimal});
    `check("%700t", value, {{83{" "}}, decimal});
    `check("%-700t", value, {decimal, {83{" "}}});

    if (!failed) $display("PASSED");
  end
endmodule
