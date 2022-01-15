/*
 * This is a reduced example from comp1001 to demonstrate a problem
 * in the Icarus Verilog code generator. If the left && argument is
 * replaced with a single 1'b1 which should be logically equivalent
 * this will work correctly. It appears that the width of the
 * expression is being calculated incorrectly.
 */
module top;
  reg [119:110] r163;
  reg [192:162] r222;

  initial begin
    r163 = 10'h17d;

    r222 = (1'b1 + (22'h3a15 && ((^r163) < 4'h8))) != 1'bx;

     // ... the subexpression ^r163 is the 1-bit value 1'b1
     //  = (1'b1 + (22'h3a15 && ((1'b1) < 4'h8))) != 1'bx
     // ... the operands of && are self determined, but the widths of the
     //     operands of < must match
     //  = (1'b1 + (22'h3a15 && (4'h1 < 4'h8))) != 1'bx
     // ... The && is a 1'bit result.
     //  = (1'b1 + 1'b1) != 1'bx
     // ... Operands of != are sized to max of i and j, namely 1 in this case.
     //  = 1'b0 != 1'bx
     //  = 1'bx
     // ... but the result is 31 bits, so the result is...
     //  = 31'b0x

    if (r222 !== 31'b0x) $display("FAILED -- r222=%b", r222);
    else $display("PASSED");
  end
endmodule
