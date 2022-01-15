module top;
  reg [3:0] val;

  initial begin
    val = 4'b1111;
    // The 'b0 should have a minimum size of integer width. This implies
    // that val should be zero extended before it is inverted. Making
    // this a false expression. See 1364-2005 (3.5.1 for width and 5.4
    // for how the width is propagated.
    if (~val == 'b0) $display("Failed.");
    else $display("PASSED");
  end
endmodule
