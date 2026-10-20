// Check whole and selected function return values passed to VPI system tasks.

module test;

  reg failed;
  reg [7:0] seed, result;
  integer index;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function [7:0] calc(input [7:0] value, input integer sel);
    reg [15:0] text;
    begin
      calc = value;
      $sformat(text, "%h", calc);
      `check(text, "a5");
      $sformat(text, "%h", calc[3:0]);
      `check(text, "5");
      $sformat(text, "%h", calc[sel +: 4]);
      if (sel == 0) begin
        `check(text, "5");
      end else begin
        `check(text, "a");
      end
    end
  endfunction

  initial begin
    failed = 1'b0;
    seed = 8'ha5;
    index = 0;
    result = calc(seed, index);
    `check(result, seed);
    index = 4;
    result = calc(seed, index);
    `check(result, seed);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
