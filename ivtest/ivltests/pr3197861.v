`begin_keywords "1364-2005"
module top;
  reg [5:0] ivar;
  real var;

  initial begin
    ivar = 0;
    var = 157.0;
    // The following line is not being calculated correctly!
    var = var - 180*ivar[5];
    if (var != 157.0) $display("Failed: This should be 157.0: ", var);
    else $display("PASSED");
  end
endmodule
`end_keywords
