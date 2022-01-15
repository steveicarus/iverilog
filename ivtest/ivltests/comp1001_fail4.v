/*
 * This is a reduced example from comp1001 to demonstrate a problem
 * in the Icarus Verilog code generator. If one addition argument is
 * replaced with a 1-bit register (instead of the constant 1'b1),
 * evaluation is postponed to vvp, which works correctly.  It appears
 * that the width of the adder is calculated incorrectly when part
 * of a comparison, but only in constant-propagation mode.
 */
module top;
  reg [30:0] r2;
  reg r1=1;

  initial begin
    r2 = (1'b1+1'b1) != 1'bx;
    // r2 = (1'b1+r1) != 1'bx;
    $displayb("r2 = ",r2);

    if (r2 !== 31'b0x) $display("FAILED");
    else $display("PASSED");
  end
endmodule
