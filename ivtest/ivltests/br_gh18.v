// Regression test for GitHub issue 18 : Icarus does undef propagation of
// const multiplies incorrectly.

module bug();

wire [3:0] y = 4'b0 * 4'bx;

initial begin
  #0 $display("%b", y);
  if (y === 4'bxxxx)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
