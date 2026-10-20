// Check compound assignments to dynamically selected function return bits.

module test;

  bit failed = 1'b0;
  integer index;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function [7:0] calc(input integer sel);
    calc = 8'ha5;
    calc[sel +: 4] += 1;
  endfunction

  initial begin
    index = 0;
    `check(calc(index), 8'ha6);
    index = 4;
    `check(calc(index), 8'hb5);
    index = 8;
    `check(calc(index), 8'ha5);
    index = 'bx;
    `check(calc(index), 8'ha5);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
