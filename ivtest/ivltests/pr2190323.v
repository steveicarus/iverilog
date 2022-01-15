/*
 * This is a reduced example from comp1001 to demonstrate a problem
 * in Icarus Verilog. Since this fails using just the compiler this
 * appears to be a problem in the elaboration of the expression.
 *
 * The division should be done at the L-value width not at the
 * argument width. This is not the case for this example.
 */
module compl1001;
  reg [133:124]r66;

  initial begin
//    r66 = ((!1'b1) / ((18'h0 - (1'b1 + 1'b1)) <= 10'h000));
    r66 = (!1'b1) / 1'b0;  // This fails.
//    r66 = 1'b0 / 1'b0;  // This passes.
    if (r66 !== 10'bx) $display("FAILED");
    else $display("PASSED");
  end
endmodule
