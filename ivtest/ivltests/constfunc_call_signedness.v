// Check that constant function calls preserve contextual signedness.

module test;

  function signed [7:0] value(input unused);
    value = -8'sd2;
  endfunction

  function [7:0] nested(input unused);
    nested = (value(unused) >>> 1) + 8'b0;
  endfunction

  localparam [7:0] DIRECT = (value(0) >>> 1) + 8'b0;
  localparam [7:0] NESTED = nested(0);
  localparam [7:0] SIGNED = value(0) >>> 1;
  localparam [7:0] WIDE = (value(0) >>> 1) + 9'b0;

  reg argument;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    argument = 1'b0;
    `check(DIRECT, 8'h7f);
    `check(NESTED, 8'h7f);
    `check(SIGNED, 8'hff);
    `check(WIDE, 8'h7f);
    `check((value(argument) >>> 1) + 8'b0, 8'h7f);
    `check(nested(argument), 8'h7f);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
