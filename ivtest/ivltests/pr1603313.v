/*
 * This program is explicitly placed in the public domain for any uses
 * whatsoever.
 */

module TestMultiplier();

  reg clk;
  initial begin
    clk = 0;
    forever #0.5 clk = ~clk;
  end

  reg[5:0] left, right;
  wire[2:0] exp;
  Multiplier mul(clk, left, right, exp);

  parameter ONE = {3'b011, 3'b0}; // 1.000 * 2**(3 - bias of 3) == 1.000

  always @ (posedge clk) begin
    left = ONE;
    right = ONE;

    #10

    if (exp !== 3'b011)
      $display("FAIL: expected %b, got %b",
               3'b011, exp);
    else
      $display("PASSED");

    $finish();
  end
endmodule


/**
 * A little bit of an incomplete floating-point multiplier.  In/out format is
 * [5:3] specify biased exponent (and hidden bit), [2:0] specify fraction.
 *
 * @param left[5:0], right[5:0]
 *   values being multiplied
 * @param exp[2:0]
 *   exponent from product of left and right when put in the floating-point
 *   format of left/right
 */
module Multiplier(clk,
                  left, right,
                  exp);
  input clk;
  input[5:0] left, right;

  output[2:0] exp;
  reg[2:0] exp;


  // IMPLEMENTATION

  wire signed[2:0] expl = left[5:3] - 3;
  wire signed[2:0] expr = right[5:3] - 3;

  /** Sum of unbiased exponents in operands. */
  reg signed[3:0] sumExp;


  always @ (posedge clk) begin




    sumExp <= (expl + expr) < -2      // why can't I move -2 to the right-hand side?

            ? -3
            : expl + expr;

    exp[2:0] <= sumExp + 3;
  end
endmodule
