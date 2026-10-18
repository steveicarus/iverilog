// Check signed division and arithmetic right shift assignments to unpacked array
// elements at run time and in constant functions. Unsigned divisors, part-selects
// and concatenations must make division unsigned; >>= must shift in zeroes.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  logic signed [3:0] values[0:1];
  integer index;

  function [3:0] update(input [3:0] initial_value);
    logic signed [3:0] local_values[0:1];
    local_values[1] = initial_value;
    local_values[1] /= 4'sd3;
    local_values[1] >>>= 4'd1;
    update = local_values[1];
  endfunction

  localparam [3:0] folded = update(-6);

  initial begin
    values[1] = -6;
    values[1] /= 4'sd3;
    `check(values[1], -4'sd2);
    index = 1;
    values[index] >>>= 4'd1;
    `check(values[1], -4'sd1);
    `check(folded, -4'sd1);
    `check(update(-6), -4'sd1);

    values[1] = -6;
    values[index] /= 4'd3;
    `check(values[1], 4'd3);
    values[1] = -6;
    values[index] >>= 4'd1;
    `check(values[1], 4'd5);
    values[1] = -6;
    values[index][3:0] /= 4'sd3;
    `check(values[1], 4'd3);
    values[1] = -6;
    {values[index]} /= 4'sd3;
    `check(values[1], 4'd3);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
